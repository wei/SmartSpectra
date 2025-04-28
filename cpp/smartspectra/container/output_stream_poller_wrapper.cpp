//
// Created by greg on 2/22/24.
// Copyright (c) 2024 Presage Technologies
//

// === standard library includes (if any) ===
// === third-party includes (if any) ===
// === local includes (if any) ===
#include "output_stream_poller_wrapper.hpp"

namespace presage::smartspectra::container::output_stream_poller_wrapper {

OutputStreamPollerWrapper::OutputStreamPollerWrapper() {
    this->stream_poller = static_cast<mediapipe::OutputStreamPoller*>(malloc(sizeof(mediapipe::OutputStreamPoller)));
}

OutputStreamPollerWrapper::~OutputStreamPollerWrapper() {
    free(this->stream_poller);
}

absl::Status OutputStreamPollerWrapper::Initialize(mediapipe::CalculatorGraph& graph, const std::string& stream_name) {
    MP_ASSIGN_OR_RETURN(mediapipe::OutputStreamPoller stream_poller, graph.AddOutputStreamPoller(stream_name));
    assert(this->stream_poller != nullptr);
    const size_t expected_size = sizeof(mediapipe::OutputStreamPoller);
    assert(sizeof (*this->stream_poller) == expected_size);
    static_assert(sizeof (stream_poller) == expected_size);
    memcpy(this->stream_poller, &stream_poller, expected_size);
    return absl::OkStatus();
}

mediapipe::OutputStreamPoller& OutputStreamPollerWrapper::Get() {
    return *this->stream_poller;
}

} // namespace presage::smartspectra::container::output_stream_poller_wrapper
