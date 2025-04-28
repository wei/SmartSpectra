//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// packet_helpers.h
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
// === standard library includes (if any) ===
// === third-party includes (if any) ===
#include <mediapipe/framework/output_stream_poller.h>
#include <physiology/modules/messages/status.h>
// === local includes (if any) ===

namespace presage::smartspectra::container::packet_helpers {

template<typename TPacketContentsType, typename TReportPredicate, bool TPrintTimestamp = false>
inline absl::Status GetPacketContentsIfAny(
    TPacketContentsType& contents,
    bool& nonempty_packet_received,
    mediapipe::OutputStreamPoller& poller,
    const char* stream_name,
    TReportPredicate&& report_if
) {
    nonempty_packet_received = false;
    mediapipe::Packet packet;
    if (poller.QueueSize() > 0) {
        if (!poller.Next(&packet)) {
            return absl::UnknownError(
                "Failed to get packet from output stream " + std::string(stream_name) + ".");
        } else {
            if (!packet.IsEmpty()) {
                nonempty_packet_received = true;
                contents = packet.Get<TPacketContentsType>();
                std::string extra_information = "";
                if (TPrintTimestamp) {
                    auto timestamp = packet.Timestamp();
                    extra_information = " (timestamp: " + std::to_string(timestamp.Value()) + ")";
                }
                if (report_if()) {
                    LOG(INFO) << "Got " + std::string(stream_name) + " packet: " << contents << extra_information;
                }
            }
        }
    }
    return absl::OkStatus();
}

template<typename TPacketContentsType>
inline absl::Status GetPacketContentsIfAny(
    TPacketContentsType& contents,
    bool& nonempty_packet_received,
    mediapipe::OutputStreamPoller& poller,
    const char* string_name,
    bool report_on_package_retrieval
) {
    return GetPacketContentsIfAny(
        contents, nonempty_packet_received, poller, string_name, [&report_on_package_retrieval]() {
            return report_on_package_retrieval;
        }
    );
}

} // namespace presage::smartspectra::container::packet_helpers
