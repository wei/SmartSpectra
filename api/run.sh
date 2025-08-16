#!/bin/bash

# SmartSpectra API Wrapper Run Script
# This script runs the SmartSpectra API wrapper server with proper configuration

set -e  # Exit on any error

# Colors for output
RED='\033[0;31m'
GREEN='\033[0;32m'
YELLOW='\033[1;33m'
BLUE='\033[0;34m'
NC='\033[0m' # No Color

# Default configuration
DEFAULT_PORT=8080
DEFAULT_HOST="0.0.0.0"
DEFAULT_MAX_SESSIONS=100
BUILD_DIR=${BUILD_DIR:-build}

echo -e "${BLUE}SmartSpectra API Wrapper Server${NC}"
echo "================================="

# Function to print status messages
print_status() {
    echo -e "${GREEN}[INFO]${NC} $1"
}

print_warning() {
    echo -e "${YELLOW}[WARNING]${NC} $1"
}

print_error() {
    echo -e "${RED}[ERROR]${NC} $1"
}

# Check if executable exists
EXECUTABLE="$BUILD_DIR/smartspectra_api_server"
if [ ! -f "$EXECUTABLE" ]; then
    print_error "Server executable not found at $EXECUTABLE"
    print_error "Please build the project first using ./build.sh"
    exit 1
fi

# Check for API key
if [ -z "$SMARTSPECTRA_API_KEY" ]; then
    print_error "SMARTSPECTRA_API_KEY environment variable is not set!"
    echo ""
    echo "Please set your SmartSpectra API key:"
    echo "  export SMARTSPECTRA_API_KEY=\"your_api_key_here\""
    echo ""
    echo "Or run with the key inline:"
    echo "  SMARTSPECTRA_API_KEY=\"your_key\" $0"
    exit 1
fi

# Configuration from environment variables
API_SERVER_PORT=${API_SERVER_PORT:-$DEFAULT_PORT}
API_SERVER_HOST=${API_SERVER_HOST:-$DEFAULT_HOST}
API_MAX_SESSIONS=${API_MAX_SESSIONS:-$DEFAULT_MAX_SESSIONS}

print_status "Configuration:"
echo "  Host: $API_SERVER_HOST"
echo "  Port: $API_SERVER_PORT"
echo "  Max Sessions: $API_MAX_SESSIONS"
echo "  API Key: ${SMARTSPECTRA_API_KEY:0:8}..." # Show only first 8 characters
echo ""

# Export configuration for the server
export API_SERVER_PORT
export API_SERVER_HOST
export API_MAX_SESSIONS

# Check if port is available
if command -v netstat &> /dev/null; then
    if netstat -tuln | grep -q ":$API_SERVER_PORT "; then
        print_warning "Port $API_SERVER_PORT appears to be in use"
        read -p "Continue anyway? (y/N): " -n 1 -r
        echo
        if [[ ! $REPLY =~ ^[Yy]$ ]]; then
            exit 1
        fi
    fi
fi

print_status "Starting SmartSpectra API server..."
echo "Server will be available at: http://$API_SERVER_HOST:$API_SERVER_PORT"
echo "Health check: http://$API_SERVER_HOST:$API_SERVER_PORT/health"
echo "API documentation: http://$API_SERVER_HOST:$API_SERVER_PORT/docs (if available)"
echo ""
echo "Sample client: Open api/samples/js/index.html in your browser"
echo ""
echo "Press Ctrl+C to stop the server"
echo ""

# Function to handle cleanup on exit
cleanup() {
    print_status "Shutting down server..."
    exit 0
}

# Set up signal handlers
trap cleanup SIGINT SIGTERM

# Run the server
exec "$EXECUTABLE"
