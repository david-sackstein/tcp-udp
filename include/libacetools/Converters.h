#pragma once

#include <common/Endpoint.h>

#include <ace/INET_Addr.h>

#include <chrono>

inline ACE_INET_Addr to_ace_addr(const Endpoint& endpoint) {
    return {endpoint.port, endpoint.address.c_str()};
}

inline Endpoint to_endpoint(const ACE_INET_Addr& ace_addr) {
    return Endpoint(ace_addr.get_host_addr(), static_cast<uint16_t>(ace_addr.get_port_number()));
}

inline ACE_Time_Value to_ace_timeout(std::chrono::milliseconds timeout) {
    return ACE_Time_Value(0, static_cast<long>(timeout.count()) * 1000); // NOLINT(*-narrowing-conversions)
}
