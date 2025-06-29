#pragma once

#include <common/Exported.h>
#include <common/Endpoint.h>
#include <libtcp/ITcpSession.h>

#include <memory>

namespace tcp {
    class ITcpClient {
    public:
        virtual ~ITcpClient() = default;

        virtual std::shared_ptr<ITcpSession> connect(const Endpoint &local, const Endpoint &remote) = 0;

        virtual void disconnect() = 0;
    };
}