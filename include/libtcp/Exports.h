#pragma once

#include <liblogger/ILogger.h>
#include <libtcp/client/ITcpClient.h>
#include <libtcp/server/ITcpClientHandler.h>
#include "common/server/IBackgroundServer.h"
#include "common/server/IBlockingServer.h"

#include <memory>
#include <vector>

namespace tcp {

EXPORTED std::unique_ptr<ITcpClient> create_tcp_client(logger::ILogger& logger);

EXPORTED std::unique_ptr<ITcpClientHandler> create_tcp_echo_handler(logger::ILogger& logger);

EXPORTED std::unique_ptr<IBlockingServer> create_tcp_server(logger::ILogger& logger,
    const Endpoint& local_endpoint,
    ITcpClientHandler& handler);

EXPORTED std::unique_ptr<IBackgroundServer> start_tcp_server(logger::ILogger& logger,
    const Endpoint& local_endpoint,
    ITcpClientHandler& handler);

EXPORTED const std::vector<std::string>& get_local_ipv4_addresses();

} // namespace tcp