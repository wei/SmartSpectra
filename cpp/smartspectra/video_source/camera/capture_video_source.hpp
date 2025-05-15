//
// Created by greg on 2/29/24.
// Copyright (c) 2024 Presage Technologies
//

#pragma once
// === standard library includes (if any) ===
// === third-party includes (if any) ===
#include <mediapipe/framework/port/opencv_video_inc.h>
#include <absl/status/status.h>
// === local includes (if any) ===
#include <smartspectra/video_source/interface.hpp>
#include <smartspectra/video_source/settings.hpp>


namespace presage::smartspectra::video_source::capture {

class CaptureVideoFileSource : public VideoSourceInterface{
public:
    absl::Status Initialize(const VideoSourceSettings& settings) override;
    CaptureVideoFileSource& operator>>(cv::Mat& frame) override;
    bool SupportsExactFrameTimestamp() const override;
    int64_t GetFrameTimestamp() const override;
    int GetWidth() override;
    int GetHeight() override;
protected:
    cv::VideoCapture capture;
};

class CaptureVideoAndTimeStampFile : public CaptureVideoFileSource {
public:
    absl::Status Initialize(const VideoSourceSettings& settings) override;
    int64_t GetFrameTimestamp() const override;
    bool SupportsExactFrameTimestamp() const override;
private:
    std::vector<int64_t> ReadTimestampsFromFile(const std::string& filename);
    std::vector<int64_t> timestamps;
};

class CaptureCameraSource :  public VideoSourceInterface{
public:
    absl::Status Initialize(const VideoSourceSettings& settings) override;
    CaptureCameraSource& operator>>(cv::Mat& frame) override;
    bool SupportsExactFrameTimestamp() const override;
    int64_t GetFrameTimestamp() const override;

    absl::Status TurnOnAutoExposure() override;
    absl::Status TurnOffAutoExposure() override;
    absl::Status ToggleAutoExposure() override;
    absl::StatusOr<bool> IsAutoExposureOn() override;
    absl::Status IncreaseExposure() override;
    absl::Status DecreaseExposure() override;
    bool SupportsExposureControls() override;

    void UseNoTimestampConversion();
    void UseUptimeTimestampConversion();

    int GetWidth() override;
    int GetHeight() override;

private:
    std::function<int64_t (int64_t input_timestamp_ms)> convert_timestamp_ms =
        [](int64_t input_timestamp_ms) { return input_timestamp_ms; };
    absl::StatusOr<double> GetExposure();
    absl::Status ModifyExposure(int by);
    cv::VideoCapture capture;
    presage::camera::AutoExposureConfiguration auto_exposure_configuration;
    bool capture_supports_timestamp = false;
    int exposure_step = 10;
};

} // namespace presage::smartspectra::video_source::capture
