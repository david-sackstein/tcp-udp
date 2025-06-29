#pragma once

#include <libudp/server/IUdpClientHandler.h>

#include <common/Buffer.h>
#include <common/Endpoint.h>
#include <common/Exported.h>

#include <memory>

namespace udp {
    class IUdpClient {
    public:
        virtual ~IUdpClient() = default;

        [[nodiscard]] virtual const Endpoint& get_local_endpoint() const = 0;

        virtual bool send_to(const Endpoint &remote, ConstBuffer buffer) = 0;

        virtual ssize_t receive_from(Buffer buffer, Endpoint &sender) = 0;
    };
}
