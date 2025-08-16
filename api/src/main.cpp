//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// main.cpp
// SmartSpectra API Wrapper Server
// Created for SmartSpectra API Wrapper
// Copyright (C) 2024 Presage Security, Inc.
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

// === standard library includes ===
#include <iostream>
#include <string>
#include <map>
#include <memory>
#include <mutex>
#include <cstdlib>
#include <chrono>
#include <thread>

// === third-party includes ===
#include <crow.h>
#include <nlohmann/json.hpp>
#include <uuid.h>
#include <glog/logging.h>
#include <opencv2/opencv.hpp>
#include <google/protobuf/util/json_util.h>

// === SmartSpectra SDK includes ===
#include <smartspectra/container/foreground_container.hpp>
#include <smartspectra/container/settings.hpp>

// === local includes ===
#include "FrameBufferVideoSource.hpp"

using json = nlohmann::json;
namespace spectra = presage::smartspectra;
namespace settings = presage::smartspectra::container::settings;

// === Session Management ===

struct Session {
    std::string id;
    std::shared_ptr<presage::smartspectra::api::FrameBufferVideoSource> video_source;
    std::shared_ptr<spectra::container::CpuContinuousRestForegroundContainer> container;
    crow::websocket::connection* ws_connection = nullptr;
    std::chrono::steady_clock::time_point created_at;
    std::string config_resolution;

    Session(const std::string& session_id, const std::string& resolution = "720p")
        : id(session_id)
        , ws_connection(nullptr)
        , created_at(std::chrono::steady_clock::now())
        , config_resolution(resolution)
    {
        video_source = std::make_shared<presage::smartspectra::api::FrameBufferVideoSource>();
    }
};

// Global session store
std::mutex g_session_mutex;
std::map<std::string, std::shared_ptr<Session>> g_sessions;

// === Utility Functions ===

std::string GenerateSessionId() {
    uuids::uuid_random_generator gen;
    auto id = gen();
    return uuids::to_string(id);
}

std::string GetApiKeyFromEnvironment() {
    const char* api_key = std::getenv("SMARTSPECTRA_API_KEY");
    if (api_key == nullptr) {
        LOG(ERROR) << "SMARTSPECTRA_API_KEY environment variable not set";
        return "";
    }
    return std::string(api_key);
}

void CleanupSession(const std::string& session_id) {
    std::lock_guard<std::mutex> lock(g_session_mutex);
    auto it = g_sessions.find(session_id);
    if (it != g_sessions.end()) {
        auto session = it->second;

        // Stop video source
        if (session->video_source) {
            session->video_source->Stop();
        }

        // Close WebSocket connection if still open
        if (session->ws_connection) {
            session->ws_connection->close("Session terminated");
            session->ws_connection = nullptr;
        }

        // Remove from sessions map
        g_sessions.erase(it);
        LOG(INFO) << "Session " << session_id << " cleaned up";
    }
}

void SessionTimeoutChecker() {
    const auto timeout_duration = std::chrono::minutes(5); // 5 minute timeout
    const auto check_interval = std::chrono::minutes(1); // Check every minute

    LOG(INFO) << "Session timeout checker started (timeout: " << timeout_duration.count() << " minutes)";

    while (true) {
        try {
            std::this_thread::sleep_for(check_interval);

            std::vector<std::string> sessions_to_cleanup;
            size_t total_sessions = 0;

            {
                std::lock_guard<std::mutex> lock(g_session_mutex);
                auto now = std::chrono::steady_clock::now();
                total_sessions = g_sessions.size();

                for (const auto& [session_id, session] : g_sessions) {
                    auto session_age = now - session->created_at;

                    // Clean up sessions that have no WebSocket connection and are older than timeout
                    if (!session->ws_connection && session_age > timeout_duration) {
                        sessions_to_cleanup.push_back(session_id);
                        LOG(WARNING) << "Session " << session_id << " timed out after "
                                   << std::chrono::duration_cast<std::chrono::minutes>(session_age).count()
                                   << " minutes without WebSocket connection";
                    }
                }
            }

            // Cleanup timed out sessions
            for (const auto& session_id : sessions_to_cleanup) {
                LOG(INFO) << "Cleaning up timed out session: " << session_id;
                CleanupSession(session_id);
            }

            // Log session statistics periodically
            if (total_sessions > 0) {
                LOG(INFO) << "Session status: " << total_sessions << " active sessions, "
                         << sessions_to_cleanup.size() << " cleaned up";
            }

        } catch (const std::exception& e) {
            LOG(ERROR) << "Error in session timeout checker: " << e.what();
        } catch (...) {
            LOG(ERROR) << "Unknown error in session timeout checker";
        }
    }
}

// Enhanced error handling for container operations
absl::Status InitializeContainer(std::shared_ptr<Session> session, const std::string& api_key) {
    try {
        // Initialize SmartSpectra container settings
        settings::Settings<settings::OperationMode::Continuous, settings::IntegrationMode::Rest> sdk_settings;
        sdk_settings.integration.api_key = api_key;

        // Create SmartSpectra container
        session->container = std::make_shared<spectra::container::CpuContinuousRestForegroundContainer>(sdk_settings);

        // Initialize the container
        auto init_status = session->container->Initialize();
        if (!init_status.ok()) {
            LOG(ERROR) << "Failed to initialize SmartSpectra container: " << init_status.message();
            return init_status;
        }

        LOG(INFO) << "SmartSpectra container initialized successfully for session " << session->id;
        return absl::OkStatus();

    } catch (const std::exception& e) {
        std::string error_msg = "Exception during container initialization: " + std::string(e.what());
        LOG(ERROR) << error_msg;
        return absl::InternalError(error_msg);
    } catch (...) {
        std::string error_msg = "Unknown exception during container initialization";
        LOG(ERROR) << error_msg;
        return absl::InternalError(error_msg);
    }
}

// === Server Configuration ===

struct ServerConfig {
    int port = 8080;
    std::string host = "0.0.0.0";
    bool enable_cors = true;
    int max_sessions = 100;

    void LoadFromEnvironment() {
        const char* port_env = std::getenv("API_SERVER_PORT");
        if (port_env) {
            port = std::atoi(port_env);
        }

        const char* host_env = std::getenv("API_SERVER_HOST");
        if (host_env) {
            host = std::string(host_env);
        }

        const char* max_sessions_env = std::getenv("API_MAX_SESSIONS");
        if (max_sessions_env) {
            max_sessions = std::atoi(max_sessions_env);
        }
    }
};

// === CORS Middleware ===

struct CORSHandler {
    struct context {};

    void before_handle(crow::request& req, crow::response& res, context& ctx) {
        if (req.method == crow::HTTPMethod::Options) {
            res.add_header("Access-Control-Allow-Origin", "*");
            res.add_header("Access-Control-Allow-Methods", "GET, POST, DELETE, OPTIONS");
            res.add_header("Access-Control-Allow-Headers", "Content-Type, Authorization");
            res.code = 200;
            res.end();
            return;
        }
    }

    void after_handle(crow::request& req, crow::response& res, context& ctx) {
        res.add_header("Access-Control-Allow-Origin", "*");
        res.add_header("Access-Control-Allow-Methods", "GET, POST, DELETE, OPTIONS");
        res.add_header("Access-Control-Allow-Headers", "Content-Type, Authorization");
    }
};

// === Main Function ===

int main(int argc, char** argv) {
    // Initialize logging
    google::InitGoogleLogging(argv[0]);
    FLAGS_alsologtostderr = true;

    LOG(INFO) << "Starting SmartSpectra API Wrapper Server";

    // Load server configuration
    ServerConfig config;
    config.LoadFromEnvironment();

    // Verify API key is available
    std::string api_key = GetApiKeyFromEnvironment();
    if (api_key.empty()) {
        LOG(FATAL) << "SMARTSPECTRA_API_KEY environment variable must be set";
        return 1;
    }

    LOG(INFO) << "API key loaded successfully";
    LOG(INFO) << "Server will listen on " << config.host << ":" << config.port;

    // Start session timeout checker thread
    std::thread timeout_thread(SessionTimeoutChecker);
    timeout_thread.detach();

    // Create Crow application with CORS middleware
    crow::App<CORSHandler> app;

    // Health check endpoint
    CROW_ROUTE(app, "/health")
    ([](const crow::request& req) {
        json response = {
            {"status", "healthy"},
            {"timestamp", std::chrono::duration_cast<std::chrono::seconds>(
                std::chrono::system_clock::now().time_since_epoch()).count()},
            {"version", "1.0.0"}
        };
        return crow::response(200, response.dump());
    });

    // POST /sessions - Create new analysis session
    CROW_ROUTE(app, "/sessions").methods("POST"_method)
    ([&api_key, &config](const crow::request& req) {
        try {
            // Parse request body
            json request_body;
            if (!req.body.empty()) {
                request_body = json::parse(req.body);
            }

            // Extract configuration
            std::string resolution = "720p"; // default
            if (request_body.contains("config") && request_body["config"].contains("resolution")) {
                resolution = request_body["config"]["resolution"];
            }

            // Check session limit
            {
                std::lock_guard<std::mutex> lock(g_session_mutex);
                if (g_sessions.size() >= config.max_sessions) {
                    json error_response = {
                        {"error", "Maximum number of sessions reached"},
                        {"max_sessions", config.max_sessions}
                    };
                    return crow::response(503, error_response.dump());
                }
            }

            // Generate session ID
            std::string session_id = GenerateSessionId();

            // Create new session
            auto session = std::make_shared<Session>(session_id, resolution);

            // Initialize SmartSpectra container settings
            settings::Settings<settings::OperationMode::Continuous, settings::IntegrationMode::Rest> sdk_settings;
            sdk_settings.integration.api_key = api_key;

            // Configure video source settings based on resolution
            if (resolution == "480p") {
                session->video_source->SetFrameDimensions(640, 480);
            } else if (resolution == "720p") {
                session->video_source->SetFrameDimensions(1280, 720);
            } else if (resolution == "1080p") {
                session->video_source->SetFrameDimensions(1920, 1080);
            } else {
                // Default to 720p for unknown resolutions
                session->video_source->SetFrameDimensions(1280, 720);
            }

            // Initialize video source
            spectra::video_source::VideoSourceSettings vs_settings;
            auto status = session->video_source->Initialize(vs_settings);
            if (!status.ok()) {
                LOG(ERROR) << "Failed to initialize video source: " << status.message();
                json error_response = {
                    {"error", "Failed to initialize video source"},
                    {"details", status.message()}
                };
                return crow::response(500, error_response.dump());
            }

            // Create SmartSpectra container
            try {
                session->container = std::make_shared<spectra::container::CpuContinuousRestForegroundContainer>(sdk_settings);
            } catch (const std::exception& e) {
                LOG(ERROR) << "Failed to create SmartSpectra container: " << e.what();
                json error_response = {
                    {"error", "Failed to create analysis container"},
                    {"details", e.what()}
                };
                return crow::response(500, error_response.dump());
            }

            // Store session
            {
                std::lock_guard<std::mutex> lock(g_session_mutex);
                g_sessions[session_id] = session;
            }

            LOG(INFO) << "Created session " << session_id << " with resolution " << resolution;

            // Prepare response
            json response = {
                {"session_id", session_id},
                {"stream_url", "ws://localhost:" + std::to_string(config.port) + "/streams/" + session_id},
                {"config", {
                    {"resolution", resolution}
                }},
                {"created_at", std::chrono::duration_cast<std::chrono::seconds>(
                    session->created_at.time_since_epoch()).count()}
            };

            return crow::response(201, response.dump());

        } catch (const json::parse_error& e) {
            LOG(ERROR) << "JSON parse error: " << e.what();
            json error_response = {
                {"error", "Invalid JSON in request body"},
                {"details", e.what()}
            };
            return crow::response(400, error_response.dump());
        } catch (const std::exception& e) {
            LOG(ERROR) << "Unexpected error creating session: " << e.what();
            json error_response = {
                {"error", "Internal server error"},
                {"details", e.what()}
            };
            return crow::response(500, error_response.dump());
        }
    });

    // DELETE /sessions/{session_id} - Delete analysis session
    CROW_ROUTE(app, "/sessions/<string>").methods("DELETE"_method)
    ([](const crow::request& req, const std::string& session_id) {
        try {
            std::lock_guard<std::mutex> lock(g_session_mutex);
            auto it = g_sessions.find(session_id);

            if (it == g_sessions.end()) {
                json error_response = {
                    {"error", "Session not found"},
                    {"session_id", session_id}
                };
                return crow::response(404, error_response.dump());
            }

            auto session = it->second;

            // Stop video source
            if (session->video_source) {
                session->video_source->Stop();
            }

            // Close WebSocket connection if still open
            if (session->ws_connection) {
                session->ws_connection->close("Session deleted");
                session->ws_connection = nullptr;
            }

            // Remove from sessions map
            g_sessions.erase(it);

            LOG(INFO) << "Deleted session " << session_id;

            return crow::response(204); // No Content

        } catch (const std::exception& e) {
            LOG(ERROR) << "Error deleting session " << session_id << ": " << e.what();
            json error_response = {
                {"error", "Internal server error"},
                {"details", e.what()}
            };
            return crow::response(500, error_response.dump());
        }
    });

    // WebSocket endpoint for streaming
    CROW_WEBSOCKET_ROUTE(app, "/streams/<string>")
    .onopen([](crow::websocket::connection& conn, const std::string& session_id) {
        LOG(INFO) << "WebSocket connection opened for session " << session_id;

        std::shared_ptr<Session> session;
        {
            std::lock_guard<std::mutex> lock(g_session_mutex);
            auto it = g_sessions.find(session_id);
            if (it == g_sessions.end()) {
                LOG(ERROR) << "Session " << session_id << " not found for WebSocket connection";
                conn.close("Session not found");
                return;
            }
            session = it->second;
        }

        // Store WebSocket connection in session
        session->ws_connection = &conn;

        // Start video source
        session->video_source->Start();

        // Set up metrics callback for the SmartSpectra container
        auto metrics_callback = [&conn, session_id](const presage::physiology::MetricsBuffer& metrics, int64_t timestamp_microseconds) -> absl::Status {
            try {
                // Convert metrics to JSON
                std::string metrics_json;
                google::protobuf::util::JsonPrintOptions options;
                options.add_whitespace = false;
                options.preserve_proto_field_names = true;

                auto status = google::protobuf::util::MessageToJsonString(metrics, &metrics_json, options);
                if (!status.ok()) {
                    LOG(ERROR) << "Failed to convert metrics to JSON: " << status.message();
                    return absl::InternalError("Failed to convert metrics to JSON");
                }

                // Create response JSON with timestamp
                json response = {
                    {"type", "metrics"},
                    {"timestamp", timestamp_microseconds},
                    {"session_id", session_id},
                    {"metrics", json::parse(metrics_json)}
                };

                // Send metrics to client
                conn.send_text(response.dump());

                return absl::OkStatus();
            } catch (const std::exception& e) {
                LOG(ERROR) << "Error in metrics callback: " << e.what();
                return absl::InternalError("Error processing metrics");
            }
        };

        // Set the metrics callback on the container
        auto callback_status = session->container->SetOnCoreMetricsOutput(metrics_callback);
        if (!callback_status.ok()) {
            LOG(ERROR) << "Failed to set metrics callback: " << callback_status.message();
            conn.close("Failed to initialize metrics callback");
            return;
        }

        // Initialize and start the container
        auto init_status = session->container->Initialize();
        if (!init_status.ok()) {
            LOG(ERROR) << "Failed to initialize container: " << init_status.message();
            conn.close("Failed to initialize analysis container");
            return;
        }

        // Start the container in a separate thread
        std::thread([session]() {
            try {
                auto run_status = session->container->Run();
                if (!run_status.ok()) {
                    LOG(ERROR) << "Container run failed: " << run_status.message();
                }
            } catch (const std::exception& e) {
                LOG(ERROR) << "Exception in container run: " << e.what();
            }
        }).detach();

        LOG(INFO) << "WebSocket connection established and container started for session " << session_id;
    })
    .onmessage([](crow::websocket::connection& conn, const std::string& data, bool is_binary, const std::string& session_id) {
        if (!is_binary) {
            LOG(WARNING) << "Received non-binary message on WebSocket for session " << session_id;
            return;
        }

        std::shared_ptr<Session> session;
        {
            std::lock_guard<std::mutex> lock(g_session_mutex);
            auto it = g_sessions.find(session_id);
            if (it == g_sessions.end()) {
                LOG(ERROR) << "Session " << session_id << " not found for frame data";
                return;
            }
            session = it->second;
        }

        try {
            // Convert binary data to cv::Mat
            std::vector<uchar> buffer(data.begin(), data.end());
            cv::Mat frame = cv::imdecode(buffer, cv::IMREAD_COLOR);

            if (frame.empty()) {
                LOG(WARNING) << "Failed to decode frame data for session " << session_id;
                return;
            }

            // Add frame to video source buffer
            if (!session->video_source->AddFrame(frame)) {
                LOG(WARNING) << "Failed to add frame to buffer for session " << session_id;
            }

        } catch (const std::exception& e) {
            LOG(ERROR) << "Error processing frame data for session " << session_id << ": " << e.what();
        }
    })
    .onclose([](crow::websocket::connection& conn, const std::string& reason, const std::string& session_id) {
        LOG(INFO) << "WebSocket connection closed for session " << session_id << ", reason: " << reason;

        // Clean up session
        CleanupSession(session_id);
    });

    LOG(INFO) << "Server initialized successfully";
    LOG(INFO) << "Available endpoints:";
    LOG(INFO) << "  GET  /health - Health check";
    LOG(INFO) << "  POST /sessions - Create new session";
    LOG(INFO) << "  DELETE /sessions/{id} - Delete session";
    LOG(INFO) << "  WebSocket /streams/{id} - Stream endpoint";

    // Start the server
    app.port(config.port).multithreaded().run();

    return 0;
}
