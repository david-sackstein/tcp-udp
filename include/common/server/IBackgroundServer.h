#pragma once

#include "common/Endpoint.h"

class IBackgroundServer {
public:
    virtual ~IBackgroundServer() = default;

    [[nodiscard]] virtual const Endpoint& get_local_endpoint() const = 0;

    virtual void stop() = 0;
};
