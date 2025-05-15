//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// foreground_container.h
// Created by Greg on 4/29/2024.
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
// === configuration header ===
#include <physiology/modules/configuration.h>
// === standard library includes (if any) ===
// === third-party includes (if any) ===
#ifdef WITH_VIDEO_OUTPUT
#include <mediapipe/framework/port/opencv_video_inc.h>
#endif
// === local includes (if any) ===
#include "container.hpp"
#include "output_stream_poller_wrapper.hpp"
#include <smartspectra/video_source/interface.hpp>

namespace presage::smartspectra::container {

template<
    platform_independence::DeviceType TDeviceType,
    settings::OperationMode TOperationMode,
    settings::IntegrationMode TIntegrationMode
>
class ForegroundContainer : public Container<TDeviceType, TOperationMode, TIntegrationMode> {
public:
    typedef container::settings::Settings<TOperationMode, TIntegrationMode> SettingsType;
    using Base = Container<TDeviceType, TOperationMode, TIntegrationMode>;
    explicit ForegroundContainer(SettingsType settings);

    absl::Status Initialize() override;
    virtual absl::Status Run();

    //TODO: add setters and make protected
    // if needed, set to a callback that handles video frames after they get preprocessed on the edge / in SDK
    std::function<absl::Status(cv::Mat& output_frame, int64_t input_timestamp)> OnVideoOutput =
        [](cv::Mat& output_frame, int64_t input_timestamp){ return absl::OkStatus(); };

protected:

    presage::smartspectra::container::output_stream_poller_wrapper::OutputStreamPollerWrapper core_metrics_poller;
    presage::smartspectra::container::output_stream_poller_wrapper::OutputStreamPollerWrapper edge_metrics_poller;

    virtual absl::Status InitializeOutputDataPollers();
    virtual absl::Status HandleOutputData(int64_t frame_timestamp);

    // state
    bool keep_grabbing_frames;
    std::unique_ptr<video_source::VideoSourceInterface> video_source = nullptr;
#ifdef WITH_VIDEO_OUTPUT
    cv::VideoWriter stream_writer;
#endif

    // settings
    const bool load_video;
private:
    void ScrollPastTimeOffset();
    static std::string GenerateGuiWindowName();
    static const std::string kWindowName;

};

typedef ForegroundContainer<platform_independence::DeviceType::Cpu, settings::OperationMode::Spot, settings::IntegrationMode::Rest> CpuSpotRestForegroundContainer;
typedef ForegroundContainer<platform_independence::DeviceType::Cpu, settings::OperationMode::Continuous, settings::IntegrationMode::Rest> CpuContinuousRestForegroundContainer;
template<settings::OperationMode TOperationMode>
using CpuRestForegroundContainer = ForegroundContainer<platform_independence::DeviceType::Cpu, TOperationMode, settings::IntegrationMode::Rest>;

typedef ForegroundContainer<platform_independence::DeviceType::Cpu, settings::OperationMode::Continuous, settings::IntegrationMode::Grpc> CpuContinuousGrpcForegroundContainer;
#ifdef WITH_OPENGL
typedef ForegroundContainer<platform_independence::DeviceType::OpenGl, settings::OperationMode::Spot, settings::IntegrationMode::Rest> OpenGlSpotRestForegroundContainer;
#endif
template<platform_independence::DeviceType TDeviceType>
using SpotRestForegroundContainer = ForegroundContainer<TDeviceType, settings::OperationMode::Spot, settings::IntegrationMode::Rest>;
} // namespace presage::smartspectra::container
