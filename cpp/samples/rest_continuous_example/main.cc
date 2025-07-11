// stdlib includes
#include <string>
#include <filesystem>
#include <fstream>

// third-party includes
#include <absl/status/status.h>
#include <absl/flags/flag.h>
#include <absl/flags/parse.h>
#include <absl/flags/usage.h>
#include <glog/logging.h>
#include <physiology/modules/configuration.h>
#include <physiology/modules/messages/metrics.h>
#include <smartspectra/container/settings.hpp>
#include <smartspectra/video_source/camera/camera.hpp>
#include <smartspectra/container/foreground_container.hpp>
#include <smartspectra/gui/opencv_hud.hpp>
#include <google/protobuf/util/json_util.h>

namespace pcam = presage::camera;
namespace spectra = presage::smartspectra;
namespace settings = presage::smartspectra::container::settings;
namespace vs = presage::smartspectra::video_source;
// region ==================================== CAMERA PARAMETERS =======================================================
//TODO: implement ABSL_FLAG_GROUP(group_name, param1, param2, param3, ...) macro in Abseil,
// which prints visually-separated, named groups of parameters/flags in help message, and use it here
ABSL_FLAG(int, camera_device_index, 0, "The index of the camera device to use in streaming capture mode.");
ABSL_FLAG(
    vs::ResolutionSelectionMode, resolution_selection_mode, vs::ResolutionSelectionMode::Auto,
    "A flag to specify the resolution selection mode when both a range and exact resolution are specified."
    "Possible values: "
    + absl::StrJoin(vs::GetResolutionSelectionModeNames(), ", ")
);
ABSL_FLAG(int, capture_width_px, -1,
          "The capture width in pixels. Set to 1280 if resolution_selection_mode is set to "
          "'auto' and no resolution range is specified.");
ABSL_FLAG(int, capture_height_px, -1,
          "The capture height in pixels. Set to 720 if resolution_selection_mode is set to "
          "'auto' and no resolution range is specified.");
ABSL_FLAG(
    pcam::CameraResolutionRange, resolution_range, pcam::CameraResolutionRange::Unspecified_EnumEnd,
    absl::StrCat(
        "The resolution range to attempt to use. Possible values: ",
        pcam::kCommonCameraResolutionRangeNameList
    )
);
ABSL_FLAG(pcam::CaptureCodec, codec, pcam::CaptureCodec::MJPG,
          absl::StrCat("Video codec to use in streaming capture mode. Possible values: ",
                       pcam::kCaptureCodecNameList));
ABSL_FLAG(bool, auto_lock, true,
          "If true, will try to use auto-exposure before recording and lock exposure when recording starts. "
          "If false, doesn't do this automatically.");
ABSL_FLAG(vs::InputTransformMode, input_transform_mode, vs::InputTransformMode::Unspecified_EnumEnd,
          absl::StrCat("Video input transformation mode. Possible values: ", vs::kInputTransformModeNameList));
ABSL_FLAG(std::string, input_video_path, "",
          "Full path of video to load. Signifies prerecorded video mode will be used. When not provided, "
          "the app will attempt to use a webcam / stream.");
ABSL_FLAG(std::string, input_video_time_path, "",
          "Full path of video timestamp txt file, "
          "where each row represents the timestamp of each frame in milliseconds.");
// endregion ===========================================================================================================
// region ======================== GUI / INTERACTION SETTINGS ==========================================================
ABSL_FLAG(bool, headless, false, "If true, no GUI will be displayed.");
ABSL_FLAG(bool, also_log_to_stderr, false, "If true, log to stderr as well.");
ABSL_FLAG(int, interframe_delay, 20,
          "Delay, in milliseconds, before capturing the next frame: "
          "higher values may free more CPU resources for the graph, giving it more time to process what it already has "
          "and drop fewer frames, resulting in more robust output metrics.");
ABSL_FLAG(bool,
          start_with_recording_on,
          false,
          "Attempt to switch data recording on at the start (even in streaming mode).");
ABSL_FLAG(int, start_time_offset_ms, 0,
          "Offset, in milliseconds, before capturing the first frame: "
          "0 starts from beginning.  30000 starts at 30s mark. "
          "Not functional for streaming mode, as start is disabled until this offset.");
// endregion ===========================================================================================================
// region ======================== GRAPH INTERNAL SETTINGS =============================================================
ABSL_FLAG(bool, scale_input, true,
          "If true, uses input scaling in the ImageTransformationCalculator within the graph.");
ABSL_FLAG(bool, enable_phasic_bp, false, "If true, enable the phasic blood pressure computation.");
ABSL_FLAG(bool, use_full_range_face_detection, false, "If true, uses the full range face detection model.");
ABSL_FLAG(bool, use_full_pose_landmarks, false, "If true, uses the full pose landmarks model.");
ABSL_FLAG(bool, enable_pose_landmark_segmentation, false, "If true, enables pose landmark segmentation.");
ABSL_FLAG(bool, enable_edge_metrics, false, "If true, enable edge metrics in the graph.");
ABSL_FLAG(bool, print_graph_contents, false, "If true, print the graph contents.");
ABSL_FLAG(bool, log_transfer_timing_info, false, "If true, log Edge<->Core transfer timing info.");
ABSL_FLAG(int, verbosity, 1, "Verbosity level -- raise to print more.");
ABSL_FLAG(std::string, api_key, "",
          "API key to use for the Physiology online service. "
          "If not provided, final features and/or metrics are not retrieved.");
// endregion ===========================================================================================================
// region ======================== CONTINUOUS-MODE SETTINGS ============================================================
ABSL_FLAG(double, buffer_duration, 0.5,
          "Duration of preprocessing buffer in seconds. Recommended values currently are between 0.2 and 1.0. "
          "Shorter values will mean more frequent updates and higher Core processing loads.");
// endregion ===========================================================================================================
// region =========================== VIDEO OUTPUT SETTINGS ============================================================
ABSL_FLAG(std::string, output_video_destination, "",
          "Full path of video to save or gstreamer output configuration string (see mode documentation). "
          "Signifies video output mode will be used. When not provided, there will be no video output "
          "(WARNING: Developer feature only. Not currently supported using the standard Physiology SDK).");
ABSL_FLAG(settings::VideoSinkMode, video_sink_mode, settings::VideoSinkMode::Unknown_EnumEnd,
          "A flag to specify the video output mode. "
          "(WARNING: Developer feature only. Not currently supported using the standard Physiology SDK). "
          "Possible values: "
          + absl::StrJoin(vs::GetResolutionSelectionModeNames(), ", ") +
          " Note that, in the `gstreamer` mode, `destination`  ");
ABSL_FLAG(bool, passthrough_video, false,
          "If true, output video will just use the input video frames directly (see destination "
          "documentation), without passing through any processing "
          "(which might contain rendered visual content from the graph).");
// endregion ===========================================================================================================
// region ========================  CUSTOM SETTINGS (not for container) ================================================
ABSL_FLAG(bool, save_metrics_to_disk, false, "If true, save metrics to disk.");
ABSL_FLAG(std::string, output_directory, "out",
          "Directory where to save acquired metrics data as JSON. "
          "If it does not exist, the app will attempt to make one.");
ABSL_FLAG(bool, enable_hud, true, "If true, enables metrics trace plotting & rate display HUD.");
ABSL_FLAG(bool, enable_framerate_diagnostics, false, "If true, enable framerate diagnostics.");
// endregion ===========================================================================================================

absl::Status RunRestContinuousEdge(
    settings::Settings<settings::OperationMode::Continuous, settings::IntegrationMode::Rest>& settings
) {
    spectra::container::CpuContinuousRestForegroundContainer container(settings);
    bool enable_hud = absl::GetFlag(FLAGS_enable_hud);
    bool enable_edge_metrics = absl::GetFlag(FLAGS_enable_edge_metrics);
    bool enable_framerate_diagnostics = absl::GetFlag(FLAGS_enable_framerate_diagnostics);
    bool save_to_disk = absl::GetFlag(FLAGS_save_metrics_to_disk);
    std::string output_directory = absl::GetFlag(FLAGS_output_directory);
    // assumes frame/output image is wider than 1270 px and taller than 410 px, adjust as needed
    spectra::gui::OpenCvHud hud(10, 0, 1260, 400);
    spectra::gui::OpenCvTracePlotter edge_metrics_plotter(10, 450, 910, 100);
    spectra::gui::OpenCvLabel edge_metrics_label(920, 450, 150, 100, "Breathing (Edge)");
    spectra::gui::OpenCvValueIndicator effective_core_fps_indicator(1200, 580, 60, 60);
    spectra::gui::OpenCvLabel effective_core_fps_label(920, 565, 270, 60, "Effective FPS (Core):");
    double effective_core_throughput = 0.0f;
    spectra::gui::OpenCvValueIndicator effective_core_latency_indicator(1200, 650, 80, 60, 3);
    spectra::gui::OpenCvLabel effective_core_latency_label(880, 635, 310, 60, "Effective latency (Core):");
    double effective_core_latency = 0.0f;


    MP_RETURN_IF_ERROR(container.SetOnStatusChange([](presage::physiology::StatusCode status_code) -> absl::Status {
        std::cout << "Imaging status: " << presage::physiology::GetStatusDescription(status_code) << std::endl;
        return absl::OkStatus();
    }));

    MP_RETURN_IF_ERROR(container.SetOnCoreMetricsOutput(
        [&settings, &hud, &enable_hud, &save_to_disk, &output_directory](
            const presage::physiology::MetricsBuffer& metrics_buffer,
            int64_t timestamp_milliseconds
        ) {
            std::string metrics_json_string;
            google::protobuf::util::JsonPrintOptions options;
            google::protobuf::util::MessageToJsonString(metrics_buffer, &metrics_json_string, options);

            if (save_to_disk) {
                if (!std::filesystem::exists(output_directory)) {
                    std::filesystem::create_directories(output_directory);
                }
                std::string output_path =
                    output_directory + std::filesystem::path::preferred_separator + "metrics_" +
                    std::to_string(timestamp_milliseconds) + ".json";
                std::ofstream output_file(output_path);
                output_file << metrics_json_string;
                output_file.close();
            }
            if (settings.verbosity_level > 1) {
                std::stringstream metrics_output;
                metrics_output << "Received metrics from Physiology Core server at timestamp "
                               << timestamp_milliseconds;
                if (settings.verbosity_level > 2) {
                    metrics_output << ": " << metrics_json_string << std::endl;
                } else {
                    metrics_output << "." << std::endl;
                }
                std::cout << metrics_output.str();
            }
            if (enable_hud) {
                hud.UpdateWithNewMetrics(metrics_buffer);
            }
            return absl::OkStatus();
        }));

    if (enable_hud) {
        MP_RETURN_IF_ERROR(container.SetOnVideoOutput(
            [&hud, &enable_edge_metrics, &edge_metrics_plotter, &edge_metrics_label,
             &enable_framerate_diagnostics,
             &effective_core_fps_indicator, &effective_core_fps_label, &effective_core_throughput,
             &effective_core_latency_indicator, &effective_core_latency_label, &effective_core_latency]
                (cv::Mat& output_frame, int64_t timestamp_milliseconds) {
                auto status = hud.Render(output_frame);
                if (!status.ok()) { return status; }
                if (enable_edge_metrics) {
                    const auto edge_color = cv::Scalar(0, 165, 255);
                    status = edge_metrics_plotter.Render(output_frame, edge_color);
                    if (!status.ok()) { return status; }
                    status = edge_metrics_label.Render(output_frame, edge_color);
                    if (!status.ok()) { return status; }
                }
                if (enable_framerate_diagnostics) {
                    const auto diagnostics_color = cv::Scalar(40, 200, 0);
                    status = effective_core_fps_indicator.Render(output_frame, effective_core_throughput, diagnostics_color);
                    if (!status.ok()) { return status; }
                    status = effective_core_fps_label.Render(output_frame, diagnostics_color);
                    if (!status.ok()) { return status; }
                    status = effective_core_latency_indicator.Render(output_frame, effective_core_latency, diagnostics_color);
                    if (!status.ok()) { return status; }
                    status = effective_core_latency_label.Render(output_frame, diagnostics_color);
                    if (!status.ok()) { return status; }
                }
                return absl::OkStatus();
            }
        ));
    }

    if (enable_edge_metrics) {
        MP_RETURN_IF_ERROR(container.SetOnEdgeMetricsOutput(
            [&settings, &edge_metrics_plotter](const presage::physiology::Metrics& metrics) {


                const auto& upper_trace = metrics.breathing().upper_trace();

                if (!upper_trace.empty()) {
#ifdef PLOT_EDGE_TRACE_ACCURATE
                    const auto& first_measurement = *upper_trace.begin();
                    if (first_measurement.stable()) {
                        edge_metrics_plotter.UpdateTraceWithSample(first_measurement);
                    }
#else
                    edge_metrics_plotter.UpdateTraceWithSample(*upper_trace.rbegin());
#endif
                }
                if (settings.verbosity_level > 2) {
                    std::string metrics_json_string;
                    google::protobuf::util::JsonPrintOptions options;
                    google::protobuf::util::MessageToJsonString(metrics, &metrics_json_string, options);
                    std::stringstream metrics_output;
                    metrics_output << "Computed new metrics on edge";
                    if (settings.verbosity_level > 3) {
                        metrics_output << ": " << metrics_json_string << std::endl;
                    } else {
                        metrics_output << "." << std::endl;
                    }
                    std::cout << metrics_output.str();
                }
                return absl::OkStatus();
            }));
    }

    if (enable_framerate_diagnostics) {
        MP_RETURN_IF_ERROR(container.SetOnCorePerformanceTelemetry(
            [&effective_core_throughput, &effective_core_latency, &enable_hud](
                double effective_core_fps,
                double effective_core_latency_seconds,
                int64_t timestamp_microseconds
            ) {
                if (!enable_hud) {
                    std::cout << "Effective Edge+Core Throughput: " << effective_core_fps << " FPS / HZ " << std::endl;
                    std::cout << "Effective Edge+Core Latency: " << effective_core_latency_seconds << " seconds" << std::endl;
                } else {
                    effective_core_throughput = effective_core_fps;
                    effective_core_latency = effective_core_latency_seconds;
                }
                return absl::OkStatus();
            }
        ));

        MP_RETURN_IF_ERROR(container.SetOnFrameSentThrough(
            [](
                bool frame_sent_through,
                int64_t timestamp_microseconds
            ) {
                if(!frame_sent_through){
                    LOG(INFO) << "Dropped frame at timestamp " << timestamp_microseconds;
                }
                return absl::OkStatus();
            }
        ));
    }

    MP_RETURN_IF_ERROR(container.Initialize());
    MP_RETURN_IF_ERROR(container.Run());

    return absl::OkStatus();
}

int main(int argc, char** argv) {
    google::InitGoogleLogging(argv[0]);

    absl::SetProgramUsageMessage(
        "Run Presage SmartSpectra C++ Rest Continuous Example on either a video file or video input from camera.\n"
        "The application will use Presage Physiology REST API to retrieve metrics continuously and plot them to the GUI "
        "(hit \"s\" to start recording metrics)."
    );
    absl::ParseCommandLine(argc, argv);

    if (absl::GetFlag(FLAGS_also_log_to_stderr)) {
        FLAGS_alsologtostderr = true;
    }

    settings::Settings<settings::OperationMode::Continuous, settings::IntegrationMode::Rest> settings{
        vs::VideoSourceSettings{
            absl::GetFlag(FLAGS_camera_device_index),
            absl::GetFlag(FLAGS_resolution_selection_mode),
            absl::GetFlag(FLAGS_capture_width_px),
            absl::GetFlag(FLAGS_capture_height_px),
            absl::GetFlag(FLAGS_resolution_range),
            absl::GetFlag(FLAGS_codec),
            absl::GetFlag(FLAGS_auto_lock),
            absl::GetFlag(FLAGS_input_transform_mode),
            absl::GetFlag(FLAGS_input_video_path),
            absl::GetFlag(FLAGS_input_video_time_path),
        },
        settings::VideoSinkSettings{
            absl::GetFlag(FLAGS_output_video_destination),
            absl::GetFlag(FLAGS_video_sink_mode),
            absl::GetFlag(FLAGS_passthrough_video)
        },
        absl::GetFlag(FLAGS_headless),
        absl::GetFlag(FLAGS_interframe_delay),
        absl::GetFlag(FLAGS_start_with_recording_on),
        absl::GetFlag(FLAGS_start_time_offset_ms),
        /*== graph internal settings ==*/
        absl::GetFlag(FLAGS_scale_input),
        /*binary_graph=*/true,
        absl::GetFlag(FLAGS_enable_phasic_bp),
        /*enable_dense_facemesh_points=*/false,
        absl::GetFlag(FLAGS_use_full_range_face_detection),
        absl::GetFlag(FLAGS_use_full_pose_landmarks),
        absl::GetFlag(FLAGS_enable_pose_landmark_segmentation),
        absl::GetFlag(FLAGS_enable_edge_metrics),
        absl::GetFlag(FLAGS_print_graph_contents),
        absl::GetFlag(FLAGS_log_transfer_timing_info),
        absl::GetFlag(FLAGS_verbosity),
        settings::ContinuousSettings{
            absl::GetFlag(FLAGS_buffer_duration)
        },
        settings::RestSettings{
            absl::GetFlag(FLAGS_api_key),
        }
    };

    absl::Status status = RunRestContinuousEdge(settings);

    if (!status.ok()) {
        LOG(ERROR) << "Run failed. " << status.message();
        return EXIT_FAILURE;
    } else {
        LOG(INFO) << "Success!";
    }

    return 0;
}
