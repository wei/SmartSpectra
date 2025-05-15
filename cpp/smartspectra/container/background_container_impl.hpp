//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// background_container_impl.h
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
#include <mediapipe/framework/formats/image_frame.h>
#include <mediapipe/framework/formats/image_frame_opencv.h>
#include <physiology/graph/stream_and_packet_names.h>
// === local includes (if any) ===
#include "background_container.hpp"
#include "image_transfer.hpp"

namespace presage::smartspectra::container {
namespace it = image_transfer;
namespace pe = physiology::edge;

template<platform_independence::DeviceType TDeviceType, settings::OperationMode TOperationMode, settings::IntegrationMode TIntegrationMode>
BackgroundContainer<TDeviceType,
    TOperationMode,
    TIntegrationMode>::BackgroundContainer(BackgroundContainer::SettingsType settings):
    Base(settings),
    graph_started(false) {}

template<platform_independence::DeviceType TDeviceType, settings::OperationMode TOperationMode, settings::IntegrationMode TIntegrationMode>
bool BackgroundContainer<TDeviceType, TOperationMode, TIntegrationMode>::GraphIsRunning() const {
    return this->graph_started;
}

template<platform_independence::DeviceType TDeviceType, settings::OperationMode TOperationMode, settings::IntegrationMode TIntegrationMode>
absl::Status BackgroundContainer<TDeviceType, TOperationMode, TIntegrationMode>::Initialize() {
    if (this->initialized) {
        LOG(INFO) << "Container already initialized, skipping initialization.";
        return absl::OkStatus();
    }
    LOG(INFO) << "Begin to initialize preprocessing container.";
    MP_RETURN_IF_ERROR(Base::Initialize());
    LOG(INFO) << "Finish preprocessing container initialization.";
    return absl::OkStatus();
}

template<platform_independence::DeviceType TDeviceType, settings::OperationMode TOperationMode, settings::IntegrationMode TIntegrationMode>
absl::Status BackgroundContainer<TDeviceType, TOperationMode, TIntegrationMode>::StartGraph() {
    if (!this->initialized) {
        return absl::FailedPreconditionError("Container not initialized.");
    }
    this->operation_context.Reset();

    if (this->OnStatusChange == nullptr){
        return absl::FailedPreconditionError(
            "OnStatusChange callback is nullptr. Expecting a valid callback. "
            "Please ensure your callback doesn't go out of scope and get destroyed while the graph is running."
        );
    }

    MP_RETURN_IF_ERROR(this->graph.ObserveOutputStream(
        pe::graph::output_streams::kStatusCode,
        [this](const mediapipe::Packet& status_packet) {
            if (!status_packet.IsEmpty()) {
                physiology::StatusCode status = status_packet.Get<physiology::StatusValue>().value();
                if (status != this->previous_status_code) {
                    this->previous_status_code = status;
                    return this->OnStatusChange(status);
                }
            }
            return absl::OkStatus();
        }
    ));

    if (this->OnCoreMetricsOutput == nullptr) {
        return absl::FailedPreconditionError(
            "OnCoreMetricsOutput callback is nullptr. Expecting a valid callback."
            "Please ensure your callback doesn't go out of scope and get destroyed while the graph is running."
        );
    }

    MP_RETURN_IF_ERROR(this->graph.ObserveOutputStream(
        physiology::edge::graph::output_streams::kMetricsBuffer,
        [this](const mediapipe::Packet& output_packet) {
            if (!output_packet.IsEmpty()) {
                auto metrics_buffer = output_packet.Get<physiology::MetricsBuffer>();
                auto timestamp = output_packet.Timestamp();
                return this->OnCoreMetricsOutput(metrics_buffer, timestamp.Value());
            }
            return absl::OkStatus();
        }
    ));

    // A separate outer if-clause used here to increase the likelihood of compiler optimizing this out
    // when we're in spot mode.
    if (TOperationMode == settings::OperationMode::Continuous){
        if (this->settings.enable_edge_metrics) {
            if (this->OnEdgeMetricsOutput == nullptr) {
                return absl::FailedPreconditionError(
                    "OnEdgeMetricsOutput callback is nullptr. Expecting a valid callback."
                    "Please ensure your callback doesn't go out of scope and get destroyed while the graph is running."
                );
            }

            MP_RETURN_IF_ERROR(this->graph.ObserveOutputStream(
                physiology::edge::graph::output_streams::kEdgeMetrics,
                [this](const mediapipe::Packet& output_packet) {
                    if (!output_packet.IsEmpty()) {
                        auto metrics_buffer = output_packet.Get<physiology::Metrics>();
                        return this->OnEdgeMetricsOutput(metrics_buffer);
                    }
                    return absl::OkStatus();
                }
            ));
        }
    }

    MP_RETURN_IF_ERROR(this->graph.StartRun({}));
    MP_RETURN_IF_ERROR(this->graph.WaitUntilIdle());
    this->graph_started = true;
    return absl::OkStatus();
}

template<platform_independence::DeviceType TDeviceType, settings::OperationMode TOperationMode, settings::IntegrationMode TIntegrationMode>
absl::Status BackgroundContainer<TDeviceType, TOperationMode, TIntegrationMode>::WaitUntilGraphIsIdle() {
    if (!this->initialized) {
        return absl::FailedPreconditionError("Container not initialized.");
    }
    if (!this->graph_started) {
        return absl::FailedPreconditionError("Graph not started.");
    }
    MP_RETURN_IF_ERROR(this->graph.WaitUntilIdle());
    return absl::OkStatus();
}

template<platform_independence::DeviceType TDeviceType, settings::OperationMode TOperationMode, settings::IntegrationMode TIntegrationMode>
absl::Status BackgroundContainer<TDeviceType, TOperationMode, TIntegrationMode>::SetRecording(bool on) {
    if (!this->initialized) {
        return absl::FailedPreconditionError("Container not initialized.");
    }
    if(!this->graph_started) {
        return absl::FailedPreconditionError("Graph not started.");
    }
    this->recording = on;
    return absl::OkStatus();
}

/**
 * Adds frame input to graph. Also, updates the recording status within the graph based on internal state of the container
 * (i.e. recording / not recording)
 * @tparam TDeviceType
 * @tparam TOperationMode
 * @tparam TIntegrationMode
 * @param frame
 * @param frame_timestamp_μs frame timestamp in microseconds; preferably, should be based on camera's own shutter clock
 * @return status of the operation
 */
template<platform_independence::DeviceType TDeviceType, settings::OperationMode TOperationMode, settings::IntegrationMode TIntegrationMode>
absl::Status BackgroundContainer<TDeviceType,
    TOperationMode,
    TIntegrationMode>::AddFrameWithTimestamp(const cv::Mat& frame_rgb, int64_t frame_timestamp_μs) {
    if (!this->initialized) {
        return absl::FailedPreconditionError("Container not initialized.");
    }
    if(!this->graph_started) {
        return absl::FailedPreconditionError("Graph not started.");
    }
    // Wrap Mat into an ImageFrame.
    auto input_frame = absl::make_unique<mediapipe::ImageFrame>(
        mediapipe::ImageFormat::SRGB, frame_rgb.cols, frame_rgb.rows,
        mediapipe::ImageFrame::kDefaultAlignmentBoundary
    );
    cv::Mat input_frame_mat = mediapipe::formats::MatView(input_frame.get());
    // transfer camera_frame data to input_frame
    frame_rgb.copyTo(input_frame_mat);

    auto frame_timestamp = mediapipe::Timestamp(frame_timestamp_μs);
    // Send recording state to the graph.
    MP_RETURN_IF_ERROR(
        this->graph
            .AddPacketToInputStream(
                pe::graph::input_streams::kRecording,
                mediapipe::MakePacket<bool>(this->recording).At(frame_timestamp)
            )
    );
    // Send image packet into the graph.
    MP_RETURN_IF_ERROR(
        it::FeedFrameToGraph(std::move(input_frame), this->graph, this->device_context, frame_timestamp_μs,
                             pe::graph::input_streams::kInputVideo)
    );
    return absl::OkStatus();
}

template<platform_independence::DeviceType TDeviceType, settings::OperationMode TOperationMode, settings::IntegrationMode TIntegrationMode>
absl::Status
BackgroundContainer<TDeviceType, TOperationMode, TIntegrationMode>::
    SetOnBluetoothCallback(std::function<absl::Status(double)> on_bluetooth) {
    if (!this->initialized) {
        return absl::FailedPreconditionError("Container not initialized.");
    }
    return this->graph.ObserveOutputStream(
        pe::graph::output_streams::kBlueTooth,
        [on_bluetooth](const mediapipe::Packet& output_packet) {
            if(!output_packet.IsEmpty()){
                auto bluetooth_timestamp = output_packet.Get<double>();
                return on_bluetooth(bluetooth_timestamp);
            }
            return absl::OkStatus();
        }
    );
}

template<platform_independence::DeviceType TDeviceType, settings::OperationMode TOperationMode, settings::IntegrationMode TIntegrationMode>
absl::Status
BackgroundContainer<TDeviceType, TOperationMode, TIntegrationMode>::
    SetOnOutputFrameCallback(std::function<absl::Status(cv::Mat&)> on_output_frame) {
    if (!this->initialized) {
        return absl::FailedPreconditionError("Container not initialized.");
    }
    return this->graph.ObserveOutputStream(
        pe::graph::output_streams::kOutputVideo,
        [this, on_output_frame](const mediapipe::Packet& output_packet) {
            if(!output_packet.IsEmpty()){
                cv::Mat output_frame_rgb;
                absl::Status status = it::GetFrameFromPacket<TDeviceType>(output_frame_rgb, this->device_context, output_packet);
                return on_output_frame(output_frame_rgb);
            }
            return absl::OkStatus();
        }
    );
}

template<platform_independence::DeviceType TDeviceType, settings::OperationMode TOperationMode, settings::IntegrationMode TIntegrationMode>
absl::Status BackgroundContainer<TDeviceType, TOperationMode, TIntegrationMode>::StopGraph() {
    if (!this->initialized) {
        return absl::FailedPreconditionError("Container not initialized.");
    }
    if (this->graph.GraphInputStreamsClosed()){
        LOG(INFO) << "Graph already stopped.";
        return absl::OkStatus();
    }
    LOG(INFO) << "Closing input streams/packet sources & stopping graph...";
    MP_RETURN_IF_ERROR(this->graph.CloseAllInputStreams());
    MP_RETURN_IF_ERROR(this->graph.CloseAllPacketSources());
    MP_RETURN_IF_ERROR(this->graph.WaitUntilDone());
    this->previous_status_code = physiology::StatusCode::PROCESSING_NOT_STARTED;
    this->graph_started = false;
    LOG(INFO) << "Graph stopped.";
    return absl::OkStatus();
}

} // namespace presage::smartspectra::container
