#pragma once

#include <common/Endpoint.h>
#include <common/Exported.h>
#include <liblogger/ILogger.h>

#include "common/server/IBackgroundServer.h"
#include "common/server/IBlockingServer.h"

#include <memory>
#include <vector>

namespace server_proxy {
EXPORTED const std::vector<std::string>& get_local_ipv4_addresses();

EXPORTED std::unique_ptr<IBlockingServer> create_tcp_server_proxy(logger::ILogger& logger,
    const Endpoint& local_endpoint,
    const Endpoint& udp_proxy);

EXPORTED std::unique_ptr<IBackgroundServer> start_tcp_server_proxy(logger::ILogger& logger,
    const Endpoint& local_endpoint,
    const Endpoint& udp_proxy);
} // namespace server_proxy