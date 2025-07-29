#pragma once

#include "ClientProxyUdpHandler.h"

#include <libudp/InternalExports.h>

#include <memory>

class TcpClientProxy final : public IBlockingServer {
public:
    TcpClientProxy(logger::ILogger& logger, const Endpoint& local_endpoint, Endpoint tcp_server);
    ~TcpClientProxy() override;

    [[nodiscard]] const Endpoint& get_local_endpoint() const override;

    void start() override;
    void stop() override;

private:
    const Endpoint local_endpoint_;
    const Endpoint tcp_server_;

    std::shared_ptr<ACE_Reactor> reactor_;
    std::unique_ptr<IBlockingServer> udp_server_;
    std::unique_ptr<ClientProxyUdpHandler> udp_handler_;
    logger::ILogger& logger_;
};