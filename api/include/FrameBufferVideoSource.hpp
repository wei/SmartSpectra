//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// FrameBufferVideoSource.hpp
// Created for SmartSpectra API Wrapper
// Copyright (C) 2024 Presage Security, Inc.
//
// This program is free software; you can redistribute it and/or
// modify it under the terms of the GNU Lesser General Public
// License as published by the Free Software Foundation; either
// version 3 of the License, or (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
// Lesser General Public License for more details.
//
// You should have received a copy of the GNU Lesser General Public License
// along with this program; if not, write to the Free Software Foundation,
// Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

#pragma once

// === standard library includes ===
#include <queue>
#include <mutex>
#include <condition_variable>
#include <atomic>
#include <chrono>

// === third-party includes ===
#include <mediapipe/framework/port/opencv_core_inc.h>
#include <absl/status/status.h>

// === SmartSpectra SDK includes ===
#include <smartspectra/video_source/video_source.hpp>

namespace presage::smartspectra::api {

/**
 * A custom video source that receives frames from WebSocket connections
 * and feeds them to the SmartSpectra SDK container.
 * 
 * This class implements a thread-safe frame buffer using a queue to store
 * incoming frames from WebSocket messages and provides them to the SDK
 * when requested.
 */
class FrameBufferVideoSource : public video_source::VideoSource {
public:
    FrameBufferVideoSource();
    virtual ~FrameBufferVideoSource();

    // VideoSource interface implementation
    absl::Status Initialize(const video_source::VideoSourceSettings& settings) override;
    bool SupportsExactFrameTimestamp() const override;
    int64_t GetFrameTimestamp() const override;
    int GetWidth() override;
    int GetHeight() override;
    video_source::InputTransformMode GetDefaultInputTransformMode() override;

    // Custom methods for WebSocket integration
    /**
     * Add a new frame to the buffer from WebSocket
     * @param frame The OpenCV Mat containing the frame data
     * @return true if frame was added successfully, false if buffer is full
     */
    bool AddFrame(const cv::Mat& frame);

    /**
     * Set the expected frame dimensions
     * @param width Frame width in pixels
     * @param height Frame height in pixels
     */
    void SetFrameDimensions(int width, int height);

    /**
     * Start the video source (allows frame production)
     */
    void Start();

    /**
     * Stop the video source (stops frame production)
     */
    void Stop();

    /**
     * Check if the video source is running
     * @return true if running, false otherwise
     */
    bool IsRunning() const;

    /**
     * Set maximum buffer size (number of frames to keep in queue)
     * @param max_size Maximum number of frames in buffer
     */
    void SetMaxBufferSize(size_t max_size);

    /**
     * Get current buffer size
     * @return Number of frames currently in buffer
     */
    size_t GetBufferSize() const;

protected:
    // VideoSource interface implementation
    void ProducePreTransformFrame(cv::Mat& frame) override;

private:
    // Frame buffer management
    std::queue<cv::Mat> frame_buffer_;
    mutable std::mutex buffer_mutex_;
    std::condition_variable frame_available_;
    
    // Video source state
    std::atomic<bool> is_running_;
    std::atomic<bool> is_initialized_;
    
    // Frame properties
    std::atomic<int> frame_width_;
    std::atomic<int> frame_height_;
    std::atomic<int64_t> current_timestamp_;
    
    // Buffer configuration
    size_t max_buffer_size_;
    
    // Timing
    std::chrono::steady_clock::time_point start_time_;
    
    // Helper methods
    int64_t GetCurrentTimestampMicroseconds() const;
    void ClearBuffer();
};

} // namespace presage::smartspectra::api
