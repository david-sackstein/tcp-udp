#pragma once

#include <common/Buffer.h>
#include <common/Endpoint.h>
#include <libacetools/IOResult.h>
#include <chrono>

namespace udp {

    class IUdpSession {
    public:
        virtual ~IUdpSession() = default;

        // pass block = std::chrono::milliseconds::max() to truly block (don't)

        virtual IOResult read_from(
            Buffer buffer,
            Endpoint& sender,
            std::chrono::milliseconds timeout) = 0;

        virtual IOResult write_to(
            ConstBuffer buffer,
            const Endpoint &remote,
            std::chrono::milliseconds timeout) = 0;
    };

}