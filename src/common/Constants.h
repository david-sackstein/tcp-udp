#pragma once

#include <chrono>
#include <cstddef>
#include <cstdint>

namespace common {

// Network constants
static constexpr uint16_t MAX_PORT = 65535;
static constexpr uint16_t DEFAULT_TCP_PORT = 12345;

// Buffer sizes
static constexpr size_t SMALL_BUFFER_SIZE = 256;
static constexpr size_t STANDARD_BUFFER_SIZE = 1024;
static constexpr size_t LARGE_BUFFER_SIZE = 2048;

// Timeout values
static constexpr auto SHORT_TIMEOUT = std::chrono::milliseconds{100};
static constexpr auto STANDARD_TIMEOUT = std::chrono::milliseconds{1000};

} // namespace common