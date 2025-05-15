//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// container.h
// Created by Greg on 2/16/2024.
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
// === standard library includes ===
#include <functional>
#include <filesystem>
// === third-party includes (if any) ===
#include <absl/status/status.h>
#include <absl/status/statusor.h>
#include <mediapipe/framework/calculator_graph.h>
#include <physiology/modules/device_type.h>
#include <physiology/modules/device_context.h>
#include <physiology/modules/messages/status.h>
#include <physiology/modules/messages/metrics.h>
// === local includes (if any) ===
#include "settings.hpp"
#include "operation_context.hpp"

namespace presage::smartspectra::container {

template<
    platform_independence::DeviceType TDeviceType,
    settings::OperationMode TOperationMode,
    settings::IntegrationMode TIntegrationMode
>
class Container {
public:
    typedef container::settings::Settings<TOperationMode, TIntegrationMode> SettingsType;

    //TODO: add setters and make protected
    // if needed, set to a callback that handles preprocessing status changes
    std::function<absl::Status(physiology::StatusCode)> OnStatusChange =
        [](physiology::StatusCode status_code){ return absl::OkStatus(); };

    //TODO: add setters and make protected
    // if needed, set to a callback that handles new metrics output from core / API
    std::function<absl::Status(const physiology::MetricsBuffer&, int64_t input_timestamp)> OnCoreMetricsOutput =
        [](const physiology::MetricsBuffer&, int64_t input_timestamp){ return absl::OkStatus(); };

    //TODO: add setters and make protected
    // if needed, set to a callback that handles new metrics output from edge / local processing
    std::function<absl::Status(const physiology::Metrics&)> OnEdgeMetricsOutput =
        [](const physiology::Metrics&){ return absl::OkStatus(); };

    explicit Container(SettingsType settings);

    virtual absl::Status Initialize();

protected:
    virtual std::string GetThirdGraphFileSuffix() const;
    virtual std::string GetGraphFilePrefix() const;
    absl::StatusOr<std::filesystem::path> GetGraphFilePath(bool binary_graph = true) const;


// ==== settings
// TODO: maybe figure out how to make `settings` `const` again?
    SettingsType settings;

// ==== state
    mediapipe::CalculatorGraph graph;
// == fixed/static after initialization
    platform_independence::DeviceContext<TDeviceType> device_context;
    bool initialized = false;
// == dynamic/changing during runtime
    physiology::StatusCode status_code = physiology::StatusCode::PROCESSING_NOT_STARTED;
    bool recording = false;
    OperationContext<TOperationMode> operation_context;
};

} // namespace presage::smartspectra::container
