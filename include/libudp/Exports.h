#pragma once

#include "client/IUdpClient.h"
#include "server/IUdpClientHandler.h"

#include <libudp/Exports.h>
#include <common/server/IBackgroundServer.h>
#include <common/server/IBlockingServer.h>

namespace udp {
    EXPORTED const std::vector<std::string>& get_local_ipv4_addresses();

    EXPORTED std::unique_ptr<IUdpClient> create_udp_client(const Endpoint& local_endpoint);

    EXPORTED std::unique_ptr<IBlockingServer> create_udp_server(
            const Endpoint& local_endpoint,
            IUdpClientHandler &handler);

    EXPORTED std::unique_ptr<IBackgroundServer> start_udp_server(
            const Endpoint& local_endpoint,
            IUdpClientHandler &handler);

    EXPORTED std::unique_ptr<IUdpClientHandler> create_udp_echo_handler();
}
