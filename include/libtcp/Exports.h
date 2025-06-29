#pragma once

#include "client/ITcpClient.h"

#include "server/ITcpClientHandler.h"

#include <common/Exported.h>
#include "common/server/IBackgroundServer.h"
#include "common/server/IBlockingServer.h"

#include <memory>

namespace tcp {
    EXPORTED const std::vector<std::string> &get_local_ipv4_addresses();

    EXPORTED std::unique_ptr<ITcpClient> create_tcp_client();

    EXPORTED std::unique_ptr<IBlockingServer> create_tcp_server(
        const Endpoint &local_endpoint,
        ITcpClientHandler &handler);

    EXPORTED std::unique_ptr<IBackgroundServer> start_tcp_server(
        const Endpoint &local_endpoint,
        ITcpClientHandler &handler);

    EXPORTED std::unique_ptr<ITcpClientHandler> create_tcp_echo_handler();
}