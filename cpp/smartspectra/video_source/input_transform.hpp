//
// Created by greg on 6/2/25.
// Copyright (c) 2025 Presage Technologies
//
// SPDX-License-Identifier: LGPL-3.0-or-later

#pragma once
// === standard library includes (if any) ===
// === third-party includes (if any) ===
#include <absl/strings/string_view.h>


namespace presage::smartspectra::video_source {

enum class InputTransformMode : int {
    None,
    Clockwise90,
    Counterclockwise90,
    Rotate180,
    MirrorHorizontal,
    MirrorVertical,
    Unspecified_EnumEnd
};

std::string AbslUnparseFlag(InputTransformMode mode);
bool AbslParseFlag(absl::string_view text, InputTransformMode* mode, std::string* error);

extern const std::vector<std::string> kInputTransformModeNames;
extern const std::string kInputTransformModeNameList;

} // namespace presage::smartspectra::video_source
