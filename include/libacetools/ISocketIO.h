#pragma once

#include "Converters.h"

#include <common/Buffer.h>
#include <common/Endpoint.h>
#include <common/IOResult.h>

#include <ace/SOCK_Stream.h>
#include <ace/SOCK_Dgram.h>
#include <ace/INET_Addr.h>

template <typename T>
Endpoint get_bound_endpoint(const T& socket) {
    ACE_INET_Addr addr;
    socket.get_local_addr(addr);
    return to_endpoint(addr);
}

class ISocketIO {
public:
    virtual ~ISocketIO() = default;

    // TCP methods
    virtual IOResult read(ACE_SOCK_Stream& socket, Buffer buffer, std::chrono::milliseconds timeout) = 0;
    virtual IOResult write(ACE_SOCK_Stream& socket, ConstBuffer buffer, std::chrono::milliseconds timeout) = 0;

    // UDP methods
    virtual IOResult read_from(ACE_SOCK_Dgram& socket, Buffer buffer, ACE_INET_Addr& sender, std::chrono::milliseconds timeout) = 0;
    virtual IOResult write(ACE_SOCK_Dgram& socket, ConstBuffer buffer, const ACE_INET_Addr& dest, std::chrono::milliseconds timeout) = 0;

    // Socket configuration
    virtual void set_linger_timeout(ACE_SOCK_Stream& socket) = 0;
};

