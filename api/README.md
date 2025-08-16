# SmartSpectra API Wrapper

A high-performance web service that wraps the SmartSpectra C++ SDK, providing a REST API for session management and WebSocket streaming for real-time physiological analysis.

## Overview

This API wrapper enables web-based applications to leverage the power of the SmartSpectra Physiology API through a simple HTTP/WebSocket interface. It implements a hybrid architecture:

- **Control Plane (REST API)**: Session management via HTTP endpoints
- **Data Plane (WebSocket)**: Real-time video streaming and metrics delivery

## Architecture

```
[Web Browser] --(REST for control)--> [C++ API Server] --(SDK Calls)--> [Physiology API]
      ^                                       ^
      |                                       |
      +----(WebSocket for data stream)--------+
```

## Features

- ✅ RESTful session management
- ✅ Real-time WebSocket video streaming
- ✅ Live physiological metrics (heart rate, respiratory rate, blink detection)
- ✅ Multiple video resolutions (480p, 720p, 1080p)
- ✅ Session timeout management
- ✅ Comprehensive error handling
- ✅ CORS support for web clients
- ✅ OpenAPI 3.0 documentation
- ✅ Sample JavaScript client

## Quick Start

### Prerequisites

- C++17 compatible compiler
- CMake 3.16+
- OpenCV 4.x
- glog
- SmartSpectra C++ SDK
- Valid SmartSpectra API key

### Build

```bash
cd api
./build.sh
```

### Run

```bash
export SMARTSPECTRA_API_KEY="your_api_key_here"
./run.sh
```

The server will start on `http://localhost:8080` by default.

### Test with Sample Client

1. Open `api/samples/js/index.html` in your web browser
2. Allow webcam access when prompted
3. Click "Start Analysis" to begin real-time analysis
4. View live physiological metrics

## API Endpoints

### REST API

| Method | Endpoint | Description |
|--------|----------|-------------|
| GET | `/health` | Health check |
| POST | `/sessions` | Create new analysis session |
| DELETE | `/sessions/{id}` | Delete analysis session |

### WebSocket

| Endpoint | Description |
|----------|-------------|
| `/streams/{session_id}` | Real-time video streaming and metrics |

## Usage Example

### 1. Create Session

```bash
curl -X POST http://localhost:8080/sessions \
  -H "Content-Type: application/json" \
  -d '{"config": {"resolution": "720p"}}'
```

Response:
```json
{
  "session_id": "123e4567-e89b-12d3-a456-426614174000",
  "stream_url": "ws://localhost:8080/streams/123e4567-e89b-12d3-a456-426614174000",
  "config": {"resolution": "720p"},
  "created_at": 1640995200
}
```

### 2. Connect WebSocket

```javascript
const ws = new WebSocket('ws://localhost:8080/streams/123e4567-e89b-12d3-a456-426614174000');
ws.binaryType = 'blob';

// Send video frames
ws.send(videoFrameBlob);

// Receive metrics
ws.onmessage = (event) => {
  const data = JSON.parse(event.data);
  console.log('Heart Rate:', data.metrics.heart_rate_bpm);
};
```

### 3. Delete Session

```bash
curl -X DELETE http://localhost:8080/sessions/123e4567-e89b-12d3-a456-426614174000
```

## Configuration

### Environment Variables

| Variable | Default | Description |
|----------|---------|-------------|
| `SMARTSPECTRA_API_KEY` | *required* | SmartSpectra API key |
| `API_SERVER_PORT` | `8080` | Server port |
| `API_SERVER_HOST` | `0.0.0.0` | Server host |
| `API_MAX_SESSIONS` | `100` | Maximum concurrent sessions |

### Video Resolutions

| Resolution | Dimensions |
|------------|------------|
| `480p` | 640x480 |
| `720p` | 1280x720 |
| `1080p` | 1920x1080 |

## Development

### Project Structure

```
api/
├── CMakeLists.txt         # Build configuration
├── build.sh              # Build script
├── run.sh                # Run script
├── src/                  # C++ source files
│   ├── main.cpp          # Main server application
│   └── FrameBufferVideoSource.cpp
├── include/              # C++ header files
│   └── FrameBufferVideoSource.hpp
├── docs/                 # API documentation
│   └── openapi.yaml      # OpenAPI specification
└── samples/              # Sample clients
    └── js/
        ├── index.html    # Sample web client
        └── app.js        # JavaScript application
```

### Building from Source

```bash
# Install dependencies (Ubuntu/Debian)
sudo apt-get install build-essential cmake pkg-config
sudo apt-get install libopencv-dev libgoogle-glog-dev

# Build
mkdir build && cd build
cmake .. -DCMAKE_BUILD_TYPE=Release
make -j$(nproc)
```

### Custom Video Source

The `FrameBufferVideoSource` class implements the SmartSpectra `VideoSource` interface:

```cpp
// Add frames from WebSocket
bool AddFrame(const cv::Mat& frame);

// Start/stop video source
void Start();
void Stop();

// Configure frame dimensions
void SetFrameDimensions(int width, int height);
```

## Security Considerations

- API key is stored server-side via environment variable
- No client-side API key exposure
- CORS enabled for web client access
- Session timeout prevents resource leaks
- Input validation on all endpoints

## Troubleshooting

### Common Issues

1. **Build fails with missing dependencies**
   - Install required packages: `libopencv-dev`, `libgoogle-glog-dev`
   - Ensure SmartSpectra SDK is properly installed

2. **Server fails to start**
   - Check that `SMARTSPECTRA_API_KEY` is set
   - Verify port 8080 is available
   - Check server logs for detailed error messages

3. **WebSocket connection fails**
   - Ensure session was created successfully
   - Check that session hasn't timed out
   - Verify WebSocket URL format

4. **No metrics received**
   - Confirm video frames are being sent correctly
   - Check that face is visible and well-lit
   - Verify SmartSpectra API key is valid

### Logging

The server uses structured logging via glog:

```bash
# Run with verbose logging
GLOG_v=2 ./run.sh

# Log to file
GLOG_log_dir=/var/log ./run.sh
```

## License

This project is licensed under the GNU Lesser General Public License v3.0.

## Docker Deployment

A Dockerfile is provided for containerized deployment:

```bash
# Build Docker image
docker build -t smartspectra-api .

# Run container
docker run -p 8080:8080 \
  -e SMARTSPECTRA_API_KEY="your_api_key_here" \
  smartspectra-api
```

## Support

For issues and questions:
- Check the troubleshooting section above
- Review server logs for error details
- Consult the OpenAPI documentation at `/docs/openapi.yaml`
