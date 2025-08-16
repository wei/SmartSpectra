//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// FrameBufferVideoSource.cpp
// Created for SmartSpectra API Wrapper
// Copyright (C) 2024 Presage Security, Inc.
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

#include "FrameBufferVideoSource.hpp"
#include <glog/logging.h>

namespace presage::smartspectra::api {

FrameBufferVideoSource::FrameBufferVideoSource()
    : is_running_(false)
    , is_initialized_(false)
    , frame_width_(640)
    , frame_height_(480)
    , current_timestamp_(0)
    , max_buffer_size_(30) // Default to 30 frames buffer
    , start_time_(std::chrono::steady_clock::now())
{
}

FrameBufferVideoSource::~FrameBufferVideoSource() {
    Stop();
    ClearBuffer();
}

absl::Status FrameBufferVideoSource::Initialize(const video_source::VideoSourceSettings& settings) {
    absl::Status status = VideoSource::Initialize(settings);
    if (!status.ok()) {
        return status;
    }
    
    is_initialized_ = true;
    LOG(INFO) << "FrameBufferVideoSource initialized successfully";
    return absl::OkStatus();
}

bool FrameBufferVideoSource::SupportsExactFrameTimestamp() const {
    return true;
}

int64_t FrameBufferVideoSource::GetFrameTimestamp() const {
    return current_timestamp_.load();
}

int FrameBufferVideoSource::GetWidth() {
    return frame_width_.load();
}

int FrameBufferVideoSource::GetHeight() {
    return frame_height_.load();
}

video_source::InputTransformMode FrameBufferVideoSource::GetDefaultInputTransformMode() {
    return video_source::InputTransformMode::None;
}

bool FrameBufferVideoSource::AddFrame(const cv::Mat& frame) {
    if (!is_running_.load() || frame.empty()) {
        return false;
    }

    std::lock_guard<std::mutex> lock(buffer_mutex_);
    
    // If buffer is full, remove oldest frame
    if (frame_buffer_.size() >= max_buffer_size_) {
        frame_buffer_.pop();
        LOG(WARNING) << "Frame buffer full, dropping oldest frame";
    }
    
    // Add new frame
    frame_buffer_.push(frame.clone());
    
    // Update frame dimensions if this is the first frame or dimensions changed
    if (frame_width_.load() != frame.cols || frame_height_.load() != frame.rows) {
        frame_width_ = frame.cols;
        frame_height_ = frame.rows;
        LOG(INFO) << "Frame dimensions updated to " << frame.cols << "x" << frame.rows;
    }
    
    // Update timestamp
    current_timestamp_ = GetCurrentTimestampMicroseconds();
    
    // Notify waiting threads
    frame_available_.notify_one();
    
    return true;
}

void FrameBufferVideoSource::SetFrameDimensions(int width, int height) {
    frame_width_ = width;
    frame_height_ = height;
    LOG(INFO) << "Frame dimensions set to " << width << "x" << height;
}

void FrameBufferVideoSource::Start() {
    is_running_ = true;
    start_time_ = std::chrono::steady_clock::now();
    LOG(INFO) << "FrameBufferVideoSource started";
}

void FrameBufferVideoSource::Stop() {
    is_running_ = false;
    frame_available_.notify_all(); // Wake up any waiting threads
    LOG(INFO) << "FrameBufferVideoSource stopped";
}

bool FrameBufferVideoSource::IsRunning() const {
    return is_running_.load();
}

void FrameBufferVideoSource::SetMaxBufferSize(size_t max_size) {
    std::lock_guard<std::mutex> lock(buffer_mutex_);
    max_buffer_size_ = max_size;
    
    // If current buffer is larger than new max size, trim it
    while (frame_buffer_.size() > max_buffer_size_) {
        frame_buffer_.pop();
    }
    
    LOG(INFO) << "Max buffer size set to " << max_size;
}

size_t FrameBufferVideoSource::GetBufferSize() const {
    std::lock_guard<std::mutex> lock(buffer_mutex_);
    return frame_buffer_.size();
}

void FrameBufferVideoSource::ProducePreTransformFrame(cv::Mat& frame) {
    if (!is_running_.load()) {
        frame = cv::Mat(); // Return empty frame if not running
        return;
    }
    
    std::unique_lock<std::mutex> lock(buffer_mutex_);
    
    // Wait for a frame to be available or for stop signal
    frame_available_.wait(lock, [this] {
        return !frame_buffer_.empty() || !is_running_.load();
    });
    
    if (!is_running_.load() || frame_buffer_.empty()) {
        frame = cv::Mat(); // Return empty frame
        return;
    }
    
    // Get the next frame
    frame = frame_buffer_.front();
    frame_buffer_.pop();
    
    // Update timestamp
    current_timestamp_ = GetCurrentTimestampMicroseconds();
}

int64_t FrameBufferVideoSource::GetCurrentTimestampMicroseconds() const {
    auto now = std::chrono::steady_clock::now();
    auto duration = std::chrono::duration_cast<std::chrono::microseconds>(now - start_time_);
    return duration.count();
}

void FrameBufferVideoSource::ClearBuffer() {
    std::lock_guard<std::mutex> lock(buffer_mutex_);
    while (!frame_buffer_.empty()) {
        frame_buffer_.pop();
    }
    LOG(INFO) << "Frame buffer cleared";
}

} // namespace presage::smartspectra::api
