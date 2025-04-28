//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// container_impl.h
// Created by Greg on 2/16/24.
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
// === configuration header ===
#include "configuration.h"
// === third-party includes (if any) ===
#include <mediapipe/framework/port/logging.h>
#include <mediapipe/framework/port/opencv_core_inc.h>
#include <mediapipe/framework/port/opencv_highgui_inc.h>
#include <mediapipe/framework/formats/image_frame.h>
#include <mediapipe/framework/formats/image_frame_opencv.h>
#include <physiology/graph/stream_and_packet_names.h>
// === local includes (if any) ===
#include "container.hpp"
#include "initialization.hpp"
#include "image_transfer.hpp"
#include "packet_helpers.hpp"
#include "benchmarking.hpp"
#include "keyboard_input.hpp"
#include "json_file_io.hpp"


namespace presage::smartspectra::container {

namespace pi = platform_independence;
namespace init = initialization;
namespace keys = keyboard_input;
namespace ph = packet_helpers;
namespace it = image_transfer;
namespace bench = benchmarking;

template<platform_independence::DeviceType TDeviceType, settings::OperationMode TOperationMode, settings::IntegrationMode TIntegrationMode>
Container<TDeviceType, TOperationMode, TIntegrationMode>::Container(Container::SettingsType settings) :
    settings(std::move(settings)),
    graph(),
    device_context(),
    operation_context(settings.operation) {};


template<
    platform_independence::DeviceType TDeviceType,
    settings::OperationMode TOperationMode,
    settings::IntegrationMode TIntegrationMode
>
absl::Status Container<TDeviceType, TOperationMode, TIntegrationMode>::Initialize() {
    if (this->initialized) {
        // Nothing to do.
        return absl::OkStatus();
    }
    // OpenCV version check needed for some video capture functions / video interface registry
    static_assert(CV_MAJOR_VERSION > 4 || (CV_MAJOR_VERSION >= 4 && CV_MINOR_VERSION >= 2),
                  "OpenCV 4.2 or above is required");

    MP_ASSIGN_OR_RETURN(std::filesystem::path graph_path, GetGraphFilePath());
    MP_RETURN_IF_ERROR(
        init::InitializeGraph<TDeviceType>(this->graph, graph_path.string(), this->settings, this->settings.binary_graph)
    );
    MP_RETURN_IF_ERROR(init::InitializeComputingDevice<TDeviceType>(this->graph, this->device_context));

    initialized = true;
    return absl::OkStatus();
}


template<platform_independence::DeviceType TDeviceType, settings::OperationMode TOperationMode, settings::IntegrationMode TIntegrationMode>
std::string Container<TDeviceType, TOperationMode, TIntegrationMode>::GetThirdGraphFileSuffix() const {
    return settings::AbslUnparseFlag(TIntegrationMode);
}


template<platform_independence::DeviceType TDeviceType, settings::OperationMode TOperationMode, settings::IntegrationMode TIntegrationMode>
std::string Container<TDeviceType, TOperationMode, TIntegrationMode>::GetGraphFilePrefix() const {
    return "metrics";
}

template<platform_independence::DeviceType TDeviceType, settings::OperationMode TOperationMode, settings::IntegrationMode TIntegrationMode>
absl::StatusOr<std::filesystem::path>
Container<TDeviceType, TOperationMode, TIntegrationMode>::GetGraphFilePath(bool binary_graph) const {
    std::string device_type = pi::AbslUnparseFlag(TDeviceType);
    std::string operation_mode = settings::AbslUnparseFlag(TOperationMode);
    std::string third_graph_suffix = this->GetThirdGraphFileSuffix();
    std::string extension = binary_graph ? ".binarypb" : ".pbtxt";
    std::string prefix = this->GetGraphFilePrefix();
    std::filesystem::path graph_directory = PHYSIOLOGY_EDGE_GRAPH_DIRECTORY;
    auto graph_file_path = graph_directory /
        (prefix + "_" + device_type + "_" + operation_mode + "_" + third_graph_suffix + extension);
    if (this->settings.verbosity_level > 1) {
        LOG(INFO) << "Retrieving graph from path: " << graph_file_path.string();
    }
    return graph_file_path;
}

} // namespace presage::smartspectra::container
