//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// background_container.h
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
// === standard library includes (if any) ===
// === third-party includes (if any) ===
#include <mediapipe/framework/port/opencv_core_inc.h>
#include <physiology/modules/messages/status.h>
#include <physiology/modules/messages/metrics.h>
// === local includes (if any) ===
#include "container.hpp"


namespace presage::smartspectra::container {

template<
    platform_independence::DeviceType TDeviceType,
    settings::OperationMode TOperationMode,
    settings::IntegrationMode TIntegrationMode
>
class BackgroundContainer : public Container<TDeviceType, TOperationMode, TIntegrationMode> {
public:
    typedef container::settings::Settings<TOperationMode, TIntegrationMode> SettingsType;
    using Base = Container<TDeviceType, TOperationMode, TIntegrationMode>;

    // constructor
    explicit BackgroundContainer(SettingsType settings);

    bool GraphIsRunning() const;
    bool ContainerIsInitialized() const { return this->initialized; };


    absl::Status Initialize() override;

    absl::Status StartGraph();

    absl::Status WaitUntilGraphIsIdle();

    absl::Status SetRecording(bool on);

    absl::Status AddFrameWithTimestamp(const cv::Mat& frame_rgb, int64_t frame_timestamp_μs);

    //TODO: move to base class (and remove from foreground container)
    //TODO: add setters and make protected
    // if needed, set to a callback that handles preprocessing status changes
    std::function<absl::Status(physiology::StatusCode)> OnStatusChange =
        [](physiology::StatusCode status_code){ return absl::OkStatus(); };

    //TODO: add setters and make protected
    // if needed, set to a callback that handles preprocessing status changes
    std::function<absl::Status(const physiology::MetricsBuffer&, int64_t input_timestamp)> OnMetricsOutputCallback =
        [](const physiology::MetricsBuffer&, int64_t input_timestamp){ return absl::OkStatus(); };

    absl::Status SetOnBluetoothCallback(std::function<absl::Status(double)> on_bluetooth);

    absl::Status SetOnOutputFrameCallback(std::function<absl::Status(cv::Mat&)> on_output_frame);

    physiology::StatusCode GetStatusCode() const { return previous_status_code; };

    absl::Status StopGraph();

private:
    // state
    bool graph_started;
    physiology::StatusCode previous_status_code = physiology::StatusCode::PROCESSING_NOT_STARTED;
};

typedef BackgroundContainer<platform_independence::DeviceType::Cpu, settings::OperationMode::Spot, settings::IntegrationMode::Rest> CpuSpotRestBackgroundContainer;
typedef BackgroundContainer<platform_independence::DeviceType::OpenGl, settings::OperationMode::Spot, settings::IntegrationMode::Rest> OpenGlSpotRestBackgroundContainer;
typedef BackgroundContainer<platform_independence::DeviceType::Cpu, settings::OperationMode::Continuous, settings::IntegrationMode::Grpc> CpuContinuousGrpcBackgroundContainer;

template<platform_independence::DeviceType TDeviceType>
using SpotRestBackgroundContainer = BackgroundContainer<TDeviceType, settings::OperationMode::Spot, settings::IntegrationMode::Rest>;

} // namespace presage::smartspectra::container
