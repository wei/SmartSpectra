//
// Created by greg on 2/29/24.
// Copyright (c) 2024 Presage Technologies
//

// === standard library includes (if any) ===
// === third-party includes (if any) ===
#include <absl/status/statusor.h>
// === local includes (if any) ===
#include "interface.hpp"

namespace presage::smartspectra::video_source {


absl::Status VideoSourceInterface::TurnOnAutoExposure() {
    return absl::UnavailableError("TurnOnAutoExposure is not supported for this VideoSource.");
}

absl::Status VideoSourceInterface::TurnOffAutoExposure() {
    return absl::UnavailableError("TurnOffAutoExposure is not supported for this VideoSource.");
}

absl::Status VideoSourceInterface::ToggleAutoExposure() {
    return absl::UnavailableError("ToggleAutoExposure is not supported for this VideoSource.");
}

absl::StatusOr<bool> VideoSourceInterface::IsAutoExposureOn() {
    return absl::UnavailableError("IsAutoExposureOn is not supported for this VideoSource.");
}

absl::Status VideoSourceInterface::IncreaseExposure() {
    return absl::UnavailableError("IncreaseExposure is not supported for this VideoSource.");
}

absl::Status VideoSourceInterface::DecreaseExposure() {
    return absl::UnavailableError("DecreaseExposure is not supported for this VideoSource.");
}

bool VideoSourceInterface::SupportsExposureControls() {
    return false;
}

int VideoSourceInterface::GetWidth() {
    return -1;
}

int VideoSourceInterface::GetHeight() {
    return -1;
}

bool VideoSourceInterface::HasFrameDimensions() {
    return this->GetHeight() > -1 && this->GetWidth() > -1;
}

} // namespace presage::smartspectra::video_source
