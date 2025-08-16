# SmartSpectra API Wrapper Dockerfile
FROM ubuntu:22.04

# Avoid interactive prompts during package installation
ENV DEBIAN_FRONTEND=noninteractive

# Install system dependencies
RUN apt-get update && apt-get install -y \
    build-essential \
    cmake \
    pkg-config \
    git \
    wget \
    curl \
    libopencv-dev \
    libgoogle-glog-dev \
    libprotobuf-dev \
    protobuf-compiler \
    libabsl-dev \
    && rm -rf /var/lib/apt/lists/*

# Set working directory
WORKDIR /app

# Copy source code
COPY . .

# Build the application
RUN mkdir -p build && cd build && \
    cmake .. -DCMAKE_BUILD_TYPE=Release && \
    make -j$(nproc)

# Create non-root user for security
RUN useradd -m -u 1000 smartspectra && \
    chown -R smartspectra:smartspectra /app

# Switch to non-root user
USER smartspectra

# Expose the default port
EXPOSE 8080

# Environment variables
ENV API_SERVER_PORT=8080
ENV API_SERVER_HOST=0.0.0.0
ENV API_MAX_SESSIONS=100

# Health check
HEALTHCHECK --interval=30s --timeout=10s --start-period=5s --retries=3 \
    CMD curl -f http://localhost:8080/health || exit 1

# Run the server
CMD ["./build/smartspectra_api_server"]
