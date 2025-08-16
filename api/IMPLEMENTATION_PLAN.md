# Project Plan: SmartSpectra API Wrapper

### 1. Objective
To create a high-performance web service that wraps the SmartSpectra C++ SDK. The service will expose a REST API for session management and use a WebSocket for real-time, bidirectional data streaming (video from client, analysis metrics to client). This will allow web-based applications to leverage the power of the SmartSpectra Physiology API.

### 2. Architecture Overview
We will implement a hybrid architecture. A C++ application will act as a proxy server between web clients and the SmartSpectra SDK.

*   **Control Plane (REST API)**: Clients will create, manage, and delete analysis sessions by making standard HTTP requests.
*   **Data Plane (WebSocket)**: For each session, a single, persistent WebSocket connection will be used for all real-time data transfer.

**Data Flow:**
```
[Web Browser] --(REST for control)--> [C++ API Server] --(SDK Calls)--> [Physiology API]
      ^                                       ^
      |                                       |
      +----(WebSocket for data stream)--------+
```

### 3. Technology Stack
*   **Server (C++)**
    *   **Language**: C++17
    *   **Build System**: CMake
    *   **Core Logic**: SmartSpectra C++ SDK
    *   **Web Library**: **Crow C++** (v1.0+) - A simple, header-only library for HTTP and WebSockets.
    *   **Dependencies**: All existing dependencies of the SmartSpectra SDK.
*   **Client (Example)**
    *   **Language**: HTML5 / JavaScript (ES6+)
    *   **APIs**: `fetch` API for REST calls, `WebSocket` API for streaming.
*   **API Documentation**
    *   **Specification**: OpenAPI 3.0 (in a `openapi.yaml` file).
    *   **Tooling**: **Redoc** or **Swagger UI** to generate a static HTML documentation site from the YAML file.

---

### 4. Security Consideration: API Key Management
This plan specifies loading the API key from an environment variable (`SMARTSPECTRA_API_KEY`) on the server. This is the recommended approach for production environments as it keeps sensitive credentials out of the source code.

*   **Security**: Storing the key on the server is the correct architectural choice for public-facing applications, as it prevents the key from being exposed to end-users in the browser.
*   **Operation**: The person deploying and running the C++ proxy server is responsible for setting this environment variable in the server's execution environment. See the 'Configuration & Deployment' section for instructions.

---

### 5. Server-Side Implementation Plan (C++ Proxy)

##### **Project Structure**
Before beginning implementation, create the following directory structure for the new proxy server project inside an `api/` folder. This plan will refer to these paths.

```
/api/
├── CMakeLists.txt         # Main CMake file for the proxy server
├── src/                   # C++ source files (.cpp)
│   ├── main.cpp
│   └── FrameBufferVideoSource.cpp
├── include/               # C++ header files (.hpp)
│   └── FrameBufferVideoSource.hpp
├── docs/                  # API documentation
│   └── openapi.yaml
└── samples/               # Sample clients
    └── js/
        ├── index.html
        └── app.js
```

##### **Phase 1: Project Setup**
1.  **Create Project Directories**: Create the full directory structure inside the `api/` folder as detailed above.
2.  **Create CMake Project**: Set up the main `CMakeLists.txt` in the `api/` directory to define the project, find dependencies, and include the `src` directory.
3.  **Link SmartSpectra SDK**: Configure CMake to find and link the installed SmartSpectra libraries and headers.
4.  **Integrate Crow**: It's recommended to add Crow as a submodule or using CMake's `FetchContent` for better dependency management.

##### **Phase 2: Core Server & Session Management**
1.  **`Session` Struct**: Define a struct to hold the state of an active session.
    ```cpp
    #include "crow_all.h"
    #include <smartspectra/container/foreground_container.hpp>

    struct Session {
        std::string id;
        std::unique_ptr<YourCustomVideoSource> video_source;
        std::unique_ptr<spectra::container::CpuContinuousRestForegroundContainer> container;
        crow::websocket::connection* ws_connection = nullptr;
    };
    ```
2.  **Session Store**: Create a global, thread-safe map for active sessions.
    ```cpp
    std::mutex g_session_mutex;
    std::map<std::string, std::shared_ptr<Session>> g_sessions;
    ```

##### **Phase 3: Custom Video Source Implementation**
1.  **`FrameBufferVideoSource` Class**: Create a class that acts as a virtual camera for the SDK.
    *   It will have an internal, thread-safe queue for `cv::Mat` objects.
    *   **`AddFrame(cv::Mat frame)`**: A public method for the WebSocket handler to push new frames into the queue.
    *   It must implement the interface required by the SDK container (e.g., `GetNextFrame()`, `IsRunning()`).

##### **Phase 4: REST Endpoint Implementation**
1.  **`POST /sessions`**
    *   **Route**: `CROW_ROUTE(app, "/sessions").methods("POST"_method)`
    *   **Request Body**:
        ```json
        {
          "config": {
            "resolution": "720p"
          }
        }
        ```
    *   **Handler Logic**:
        *   Fetch the API key from an environment variable: `### 4. Security Consideration: API Key Management
This plan specifies loading the API key from an environment variable (`SMARTSPECTRA_API_KEY`) on the server. This is the recommended approach for production environments as it keeps sensitive credentials out of the source code.

*   **Security**: Storing the key on the server is the correct architectural choice for public-facing applications, as it prevents the key from being exposed to end-users in the browser.
*   **Operation**: The person deploying and running the C++ proxy server is responsible for setting this environment variable in the server's execution environment. See the 'Configuration & Deployment' section for instructions.

---

### 5. Error Handling & Resilience
A robust server must anticipate and handle various error conditions gracefully.
*   **SDK Failures**: The server should handle cases where the SmartSpectra SDK fails to initialize or encounters an error during analysis. This should be logged clearly, and an appropriate error message (e.g., `503 Service Unavailable`) should be sent to the client.
*   **Connection Issues**: The server must be resilient to unexpected WebSocket disconnections. The `onclose` handler is the primary place to manage the cleanup of associated resources.
*   **Invalid Client Data**: The server should validate incoming data. For instance, if a WebSocket message is not a valid video frame, it should be discarded and a warning logged.
*   **Logging**: Implement structured logging throughout the application. This will be crucial for debugging issues in both development and production environments.

#### Session Timeouts
To prevent orphaned sessions from consuming server resources indefinitely, a timeout mechanism is recommended. If a client creates a session via `POST /sessions` but never connects a WebSocket, or if a WebSocket connection is dropped and not re-established within a certain period, the server should automatically clean up the session.

---

### 6. Server-Side Implementation Plan (C++ Proxy)

##### **Project Structure**
Before beginning implementation, create the following directory structure for the new proxy server project inside an `api/` folder. This plan will refer to these paths.

```
/api/
├── CMakeLists.txt         # Main CMake file for the proxy server
├── src/                   # C++ source files (.cpp)
│   ├── main.cpp
│   └── FrameBufferVideoSource.cpp
├── include/               # C++ header files (.hpp)
│   └── FrameBufferVideoSource.hpp
├── docs/                  # API documentation
│   └── openapi.yaml
└── samples/               # Sample clients
    └── js/
        ├── index.html
        └── app.js
```

##### **Phase 1: Project Setup**
1.  **Create Project Directories**: Create the full directory structure inside the `api/` folder as detailed above.
2.  **Create CMake Project**: Set up the main `CMakeLists.txt` in the `api/` directory to define the project, find dependencies, and include the `src` directory.
3.  **Link SmartSpectra SDK**: Configure CMake to find and link the installed SmartSpectra libraries and headers.
4.  **Integrate Crow**: It's recommended to add Crow as a submodule or using CMake's `FetchContent` for better dependency management.

##### **Phase 2: Core Server & Session Management**
1.  **`Session` Struct**: Define a struct to hold the state of an active session. For consistency and simpler memory management, use `std::shared_ptr` for all managed objects.
    ```cpp
    #include "crow_all.h"
    #include <smartspectra/container/foreground_container.hpp>

    struct Session {
        std::string id;
        std::shared_ptr<YourCustomVideoSource> video_source;
        std::shared_ptr<spectra::container::CpuContinuousRestForegroundContainer> container;
        crow::websocket::connection* ws_connection = nullptr;
    };
    ```
2.  **Session Store**: Create a global, thread-safe map for active sessions.
    ```cpp
    std::mutex g_session_mutex;
    std::map<std::string, std::shared_ptr<Session>> g_sessions;
    ```

##### **Phase 3: Custom Video Source Implementation**
1.  **`FrameBufferVideoSource` Class**: Create a class that acts as a virtual camera for the SDK.
    *   It will have an internal, thread-safe queue for `cv::Mat` objects.
    *   **`AddFrame(cv::Mat frame)`**: A public method for the WebSocket handler to push new frames into the queue.
    *   It must implement the interface required by the SDK container (e.g., `GetNextFrame()`, `IsRunning()`).

##### **Phase 4: REST Endpoint Implementation**
1.  **`POST /sessions`**
    *   **Route**: `CROW_ROUTE(app, "/sessions").methods("POST"_method)`
    *   **Request Body**:
        ```json
        {
          "config": {
            "resolution": "720p"
          }
        }
        ```
    *   **Handler Logic**:
        *   Fetch the API key from an environment variable: `const char* apiKey = std::getenv("SMARTSPECTRA_API_KEY");`
        *   **If the API key is not set**, log a critical error and return a `500 Internal Server Error` response.
        *   Parse the JSON body to get the session configuration (e.g., resolution). Use this to configure the SDK container.
        *   Generate a unique session ID (e.g., using a UUID library).
        *   Create and store a new `Session` object in the global map.
        *   Instantiate the `FrameBufferVideoSource` and `CpuContinuousRestForegroundContainer`.
        *   **Configure the container with the `apiKey` from the environment and settings from the request.**
        *   Return `201 Created` with the `session_id` and `stream_url`.
2.  **`DELETE /sessions/<string>`**
    *   **Route**: `CROW_ROUTE(app, "/sessions/<string>").methods("DELETE"_method)`
    *   **Handler Logic**: Find the session, close the WebSocket, stop the container, and remove it from the map. Return `204 No Content`.

##### **Phase 5: WebSocket Endpoint Implementation**
1.  **`GET /streams/<string>`**
    *   **Route**: `CROW_WEBSOCKET_ROUTE(app, "/streams/<string>")`
    *   **`onopen` handler**:
        *   Find the session by ID. If not found, close the connection.
        *   Store the `crow::websocket::connection` pointer in the `Session` struct.
        *   Set the `container->SetOnCoreMetricsOutput()` callback. The lambda must capture the connection pointer to send metrics back to the client. **Note**: The exact structure of the JSON metrics payload sent to the client should be clearly documented.
        *   Call `container->Run()` to start the analysis.
    *   **`onmessage` handler**:
        *   Receive binary video frame data, convert to `cv::Mat`, and pass it to your custom video source via `AddFrame()`.
        *   **Note**: For future extensibility, consider a more structured message format, such as a JSON header followed by the binary frame data.
    *   **`onclose` handler**:
        *   Trigger the session cleanup logic (same as the `DELETE` endpoint).

---

### 7. API Documentation
1.  **Create `openapi.yaml`**: Create a file named `openapi.yaml` inside the `api/docs/` directory. This file will contain the formal specification for the REST API.
2.  **Define Endpoints**: Document all endpoints, methods, request/response bodies, and status codes. The documentation should be comprehensive and include:
    *   The `POST /sessions` and `DELETE /sessions/{session_id}` endpoints.
    *   Detailed schemas for all JSON request and response bodies.
    *   All possible API error responses.
    *   A clear description of the WebSocket protocol, including the format of messages sent from the server (metrics) and expected from the client (video frames). While OpenAPI 3.0 has limited support for WebSockets, this can be described in the general API description or a separate Markdown document.
    *   **Example for `POST /sessions`**:
        ```yaml
        openapi: 3.0.0
        info:
          title: SmartSpectra Web Wrapper API
          version: 1.0.0
        paths:
          /sessions:
            post:
              summary: Create a new analysis session
              requestBody:
                required: true
                content:
                  application/json:
                    schema:
                      type: object
                      properties:
                        config:
                          type: object
                          properties:
                            resolution:
                              type: string
                              enum: ["480p", "720p", "1080p"]
              responses:
                '201':
                  description: Session created successfully
                  content:
                    application/json:
                      schema:
                        type: object
                        properties:
                          session_id:
                            type: string
                          stream_url:
                            type: string
        ```
3.  **Generate HTML Documentation**:
    *   Use a tool like **Redoc** to generate a single, static HTML file from the `openapi.yaml`.
    *   **Command**: From the project root, run: `npx redoc-cli build api/docs/openapi.yaml -o api/docs/api-docs.html`
    *   This `api-docs.html` file can be served alongside the application or hosted separately.

---

### 8. Client-Side Implementation Plan (JavaScript)

**Note**: The following HTML and JavaScript files should be placed in the `api/samples/js/` directory.

This plan outlines a sample client to demonstrate the API for both **Continuous** and **Spot** analysis modes. It will process a live webcam stream, display key physiological metrics, and provide visual feedback for events like blinks.

#### HTML Structure (`index.html`)
The HTML will include radio buttons to select the analysis mode.

```html
<!DOCTYPE html>
<html>
<head>
    <title>SmartSpectra Client</title>
    <style>
        #blink-indicator { transition: background-color 0.1s ease; }
        .blink { background-color: yellow !important; }
        fieldset { margin-bottom: 1em; }
    </style>
</head>
<body>
    <h1>SmartSpectra Web Client</h1>
    <video id="webcam" width="640" height="480" autoplay muted></video>
    <canvas id="canvas" style="display:none;"></canvas>

    <fieldset>
        <legend>Analysis Mode</legend>
        <input type="radio" id="modeContinuous" name="analysisMode" value="continuous" checked>
        <label for="modeContinuous">Continuous</label>
        <input type="radio" id="modeSpot" name="analysisMode" value="spot">
        <label for="modeSpot">Spot (5 seconds)</label>
    </fieldset>

    <div>
        <button id="startButton">Start Analysis</button>
        <button id="stopButton">Stop Analysis</button>
    </div>

    <h2>Live Metrics</h2>
    <div id="metrics">
        <p>Pulse: <span id="pulse">--</span> BPM</p>
        <p>Breaths: <span id="breaths">--</span> RPM</p>
        <p>Blink Indicator: <span id="blink-indicator" style="display: inline-block; width: 20px; height: 20px; border-radius: 50%; background-color: grey;"></span></p>
    </div>

    <script src="app.js"></script>
</body>
</html>
```

#### JavaScript Logic (`app.js`)

1.  **Initialization**:
    *   Get references to all DOM elements (`video`, `canvas`, buttons, metric spans, radio buttons).
    *   Initialize webcam stream using `navigator.mediaDevices.getUserMedia()` and pipe it to the `<video>` element.

2.  **`startAnalysis()`**:
    *   Attached to the "Start Analysis" button's click event.
    *   Reads the selected mode ("continuous" or "spot") from the radio buttons.
    *   Disables the UI controls to prevent multiple sessions.
    *   Calls `fetch('/sessions', {method: 'POST', ...})` to create a session on the server.
    *   On success, gets the `stream_url` and `session_id`.
    *   Calls `connect(stream_url, selectedMode)` to establish the WebSocket connection, passing the chosen mode.

3.  **`connect(url, mode)`**:
    *   Creates a `new WebSocket(url)`.
    *   Sets `ws.binaryType = "blob";`.
    *   **`ws.onopen`**:
        *   If `mode === 'continuous'`, it starts a `setInterval` loop to call `sendFrame(ws)` indefinitely.
        *   If `mode === 'spot'`, it starts a `setInterval` loop but also sets a `setTimeout` for 5 seconds. When the timeout fires, it clears the interval (to stop sending frames) and calls `stopAnalysis()` to terminate the session.
    *   **`ws.onmessage`**: This logic remains the same, updating the UI with incoming metrics.
        *   Receives and parses the JSON data from the server.
        *   Updates the pulse, breaths, and blink indicator displays.
    *   **`ws.onclose`**: Logs that the connection has been closed and re-enables the UI controls.

4.  **`sendFrame(ws)`**:
    *   This function's implementation is unchanged. It draws the video to the canvas and sends the frame as a JPEG blob.
    *   **Note**: `canvas.toBlob()` is asynchronous. For more demanding real-time applications, exploring alternatives like the `MediaStreamTrackProcessor` API might be necessary to ensure smooth frame delivery.

5.  **`stopAnalysis()`**:
    *   Attached to the "Stop Analysis" button. It is also called automatically in "Spot" mode.
    *   Calls `fetch('/sessions/SESSION_ID', {method: 'DELETE'})`.
    *   Closes the WebSocket connection if it's open: `ws.close()`. `
        *   **If the API key is not set**, log a critical error and return a `500 Internal Server Error` response.
        *   Parse the JSON body to get the session configuration.
        *   Generate a unique session ID (UUID).
        *   Create and store a new `Session` object in the global map.
        *   Instantiate the `FrameBufferVideoSource` and `CpuContinuousRestForegroundContainer`.
        *   **Configure the container with the `apiKey` from the environment.**
        *   Return `201 Created` with the `session_id` and `stream_url`.
2.  **`DELETE /sessions/<string>`**
    *   **Route**: `CROW_ROUTE(app, "/sessions/<string>").methods("DELETE"_method)`
    *   **Handler Logic**: Find the session, close the WebSocket, stop the container, and remove it from the map. Return `204 No Content`.

##### **Phase 5: WebSocket Endpoint Implementation**
1.  **`GET /streams/<string>`**
    *   **Route**: `CROW_WEBSOCKET_ROUTE(app, "/streams/<string>")`
    *   **`onopen` handler**:
        *   Find the session by ID. If not found, close the connection.
        *   Store the `crow::websocket::connection` pointer in the `Session` struct.
        *   Set the `container->SetOnCoreMetricsOutput()` callback. The lambda must capture the connection pointer to send metrics back to the client.
        *   Call `container->Run()` to start the analysis.
    *   **`onmessage` handler**:
        *   Receive binary video frame data, convert to `cv::Mat`, and pass it to your custom video source via `AddFrame()`.
    *   **`onclose` handler**:
        *   Trigger the session cleanup logic (same as the `DELETE` endpoint).

---

### 6. API Documentation
1.  **Create `openapi.yaml`**: Create a file named `openapi.yaml` inside the `api/docs/` directory. This file will contain the formal specification for the REST API.
2.  **Define Endpoints**: Document all endpoints, methods, request/response bodies, and status codes.
    *   **Example for `POST /sessions`**:
        ```yaml
        openapi: 3.0.0
        info:
          title: SmartSpectra Web Wrapper API
          version: 1.0.0
        paths:
          /sessions:
            post:
              summary: Create a new analysis session
              requestBody:
                required: true
                content:
                  application/json:
                    schema:
                      type: object
                      properties:
                        config:
                          type: object
              responses:
                '201':
                  description: Session created successfully
                  content:
                    application/json:
                      schema:
                        type: object
                        properties:
                          session_id:
                            type: string
                          stream_url:
                            type: string
        ```
3.  **Generate HTML Documentation**:
    *   Use a tool like **Redoc** to generate a single, static HTML file from the `openapi.yaml`.
    *   **Command**: From the project root, run: `npx redoc-cli build api/docs/openapi.yaml -o api/docs/api-docs.html`
    *   This `api-docs.html` file can be served alongside the application or hosted separately.

---

### 7. Client-Side Implementation Plan (JavaScript)

**Note**: The following HTML and JavaScript files should be placed in the `api/samples/js/` directory.

This plan outlines a sample client to process the live webcam stream, display key physiological metrics, and provide visual feedback for events like blinks.

#### HTML Structure (`index.html`)
```html
<!DOCTYPE html>
<html>
<head>
    <title>SmartSpectra Client</title>
    <style>
        #blink-indicator { transition: background-color 0.1s ease; }
        .blink { background-color: yellow !important; }
    </style>
</head>
<body>
    <h1>SmartSpectra Web Client</h1>
    <video id="webcam" width="640" height="480" autoplay muted></video>
    <canvas id="canvas" style="display:none;"></canvas>
    <div>
        <button id="startButton">Start Session</button>
        <button id="stopButton">Stop Session</button>
    </div>
    <h2>Live Metrics</h2>
    <div id="metrics">
        <p>Pulse: <span id="pulse">--</span> BPM</p>
        <p>Breaths: <span id="breaths">--</span> RPM</p>
        <p>Blink Indicator: <span id="blink-indicator" style="display: inline-block; width: 20px; height: 20px; border-radius: 50%; background-color: grey;"></span></p>
    </div>

    <script src="app.js"></script>
</body>
</html>
```

#### JavaScript Logic (`app.js`)

1.  **Initialization**:
    *   Get references to all DOM elements (`video`, `canvas`, buttons, metric spans).
    *   Initialize webcam stream using `navigator.mediaDevices.getUserMedia()` and pipe it to the `<video>` element.

2.  **`startSession()`**:
    *   Attached to the "Start" button's click event.
    *   Calls `fetch('/sessions', {method: 'POST', body: JSON.stringify({config: {resolution: '480p'}})})`.
    *   On success, gets the `stream_url` and `session_id` from the response.
    *   Calls `connect(stream_url)` to establish the WebSocket connection.

3.  **`connect(url)`**:
    *   Creates a `new WebSocket(url)`.
    *   Sets `ws.binaryType = "blob";` to handle binary frame data.
    *   **`ws.onopen`**: When the connection opens, it starts a `setInterval` loop that calls `sendFrame(ws)` every 100ms (for 10fps).
    *   **`ws.onmessage`**: This is the core logic for displaying results.
        *   Receives the metrics event from the server.
        *   Parses the JSON data: `const result = JSON.parse(event.data);`.
        *   **Note**: The exact field names below (`heart_rate_bpm`, `respiratory_rate_rpm`, `blink_detected`) are examples and depend on the actual data structure returned by the Physiology API.
        *   Updates the pulse display: `document.getElementById('pulse').textContent = result.metrics.heart_rate_bpm.toFixed(1);`.
        *   Updates the breaths display: `document.getElementById('breaths').textContent = result.metrics.respiratory_rate_rpm.toFixed(1);`.
        *   Handles the blink indicator:
            ```javascript
            if (result.metrics.blink_detected) {
                const indicator = document.getElementById('blink-indicator');
                indicator.classList.add('blink');
                setTimeout(() => {
                    indicator.classList.remove('blink');
                }, 200); // Blink effect lasts for 200ms
            }
            ```
    *   **`ws.onclose`**: Logs that the connection has been closed.

4.  **`sendFrame(ws)`**:
    *   Gets the canvas 2D context.
    *   Draws the current frame from the `<video>` element onto the hidden `<canvas>`.
    *   Gets the frame data as a JPEG blob: `canvas.toBlob(blob => ws.send(blob), 'image/jpeg', 0.8);`.

5.  **`stopSession()`**:
    *   Attached to the "Stop" button.
    *   Calls `fetch('/sessions/SESSION_ID', {method: 'DELETE'})`.
    *   Closes the WebSocket connection if it's open: `ws.close()`.

---

### 8. Milestones
1.  **M1: Server Setup**: CMake project created, Crow integrated, basic server runs.
2.  **M2: REST & Session Management**: `POST /sessions` and `DELETE /sessions` are functional, using an environment variable for the API key.
3.  **M3: API Documentation**: Initial `openapi.yaml` is created and `api-docs.html` is generated.
4.  **M4: WebSocket & Video Handling**: Custom video source implemented and WebSocket can receive binary frames.
5.  **M5: Full Integration**: SDK metrics callback is fully wired to the WebSocket output.
6.  **M6: Client Implementation**: A sample web client is built for end-to-end testing.

---

### 9. Configuration & Deployment

#### API Key Configuration

The C++ api server requires the SmartSpectra Physiology API key to be provided via an environment variable named `SMARTSPECTRA_API_KEY`.

Before running the server, you must set this variable in your shell or execution environment. The server will fail to start if this variable is not present.

**On Linux or macOS:**
```bash
export SMARTSPECTRA_API_KEY="your_actual_api_key_here"
./path/to/your/api_server
```
