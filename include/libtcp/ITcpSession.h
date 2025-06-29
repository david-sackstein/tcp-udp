#pragma once

#include <common/Buffer.h>
#include <common/Endpoint.h>
#include <common/IOResult.h>

#include <chrono>

namespace tcp {

    class ITcpSession {
    public:
        virtual ~ITcpSession() = default;

        [[nodiscard]] virtual Endpoint get_peer() const = 0;

        // pass block = std::chrono::milliseconds::max() to truly block (don't)

        virtual IOResult read(Buffer buffer, std::chrono::milliseconds timeout) = 0;

        virtual IOResult write(ConstBuffer buffer, std::chrono::milliseconds timeout) = 0;

        virtual void close() = 0;
    };

}
