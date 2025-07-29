#pragma once

#include "ServerProxyClientHandler.h"
#include "ServerProxyUdpHandler.h"

#include <ace/Event_Handler.h>
#include <ace/Reactor.h>
#include <libudp/client/IUdpClient.h>
#include <memory>
#include "common/server/IBackgroundServer.h"
#include "common/server/IBlockingServer.h"

class TcpServerProxy final : public IBlockingServer, public ACE_Event_Handler {
public:
    TcpServerProxy(logger::ILogger& logger, Endpoint local_endpoint, Endpoint udp_proxy);

    [[nodiscard]] const Endpoint& get_local_endpoint() const override;

    void start() override;
    void stop() override;

    // ACE_Event_Handler interface for UDP client socket
    int handle_input(ACE_HANDLE fd) override;
    [[nodiscard]] ACE_HANDLE get_handle() const override;

private:
    const Endpoint local_endpoint_;
    const Endpoint udp_proxy_;

    std::shared_ptr<ACE_Reactor> reactor_;
    std::unique_ptr<IBlockingServer> tcp_server_;
    std::unique_ptr<udp::IUdpClient> udp_client_;
    std::unique_ptr<ServerProxyClientHandler> tcp_handler_;
    std::unique_ptr<ServerProxyUdpHandler> udp_handler_;
    logger::ILogger& logger_;

    bool reactor_registered_ = false;
};