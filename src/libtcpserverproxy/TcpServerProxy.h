#pragma once

#include "TcpServerProxyClientHandler.h"

#include "common/server/IBackgroundServer.h"
#include "common/server/IBlockingServer.h"
#include <libudp/client/IUdpClient.h>

class TcpServerProxy final : public IBlockingServer {
public:
    TcpServerProxy(Endpoint local_endpoint, Endpoint udp_proxy);

    [[nodiscard]] const Endpoint& get_local_endpoint() const override;

    void start() override;
    void stop() override;

private:
    const Endpoint local_endpoint_;
    const Endpoint udp_proxy_;

    std::unique_ptr<IBlockingServer> tcp_server_;
    std::unique_ptr<udp::IUdpClient> udp_client_;
    std::unique_ptr<TcpServerProxyClientHandler> handler_;
}; 