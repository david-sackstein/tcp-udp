#pragma once

#include <common/Endpoint.h>

class IBlockingServer {
public:
    virtual ~IBlockingServer() = default;

    [[nodiscard]] virtual const Endpoint& get_local_endpoint() const = 0;

    virtual void start() = 0; // blocks the current thread

    virtual void stop() = 0; // call from another thread to end immediately
};
