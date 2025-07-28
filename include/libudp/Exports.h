#pragma once

#include "client/IUdpClient.h"
#include "server/IUdpClientHandler.h"

#include <liblogger/ILogger.h>
#include <common/server/IBackgroundServer.h>
#include <common/server/IBlockingServer.h>

#include <memory>

namespace udp {
    EXPORTED const std::vector<std::string> &get_local_ipv4_addresses();

    EXPORTED std::unique_ptr<IUdpClient> create_udp_client(logger::ILogger &logger, const Endpoint &local_endpoint);

    EXPORTED std::unique_ptr<IBlockingServer> create_udp_server(
        logger::ILogger &logger,
        const Endpoint &local_endpoint,
        IUdpClientHandler &handler);

    EXPORTED std::unique_ptr<IBackgroundServer> start_udp_server(
        logger::ILogger &logger,
        const Endpoint &local_endpoint,
        IUdpClientHandler &handler);

    EXPORTED std::unique_ptr<IUdpClientHandler> create_udp_echo_handler(logger::ILogger &logger);
}
