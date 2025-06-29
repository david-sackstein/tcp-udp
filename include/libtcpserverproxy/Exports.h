#pragma once

#include <common/Exported.h>
#include <common/Endpoint.h>
#include "common/server/IBackgroundServer.h"
#include "common/server/IBlockingServer.h"

#include <vector>
#include <memory>

namespace server_proxy {
    EXPORTED const std::vector<std::string>& get_local_ipv4_addresses();

    EXPORTED std::unique_ptr<IBlockingServer> create_tcp_server_proxy(
        const Endpoint &local_endpoint,
        const Endpoint &udp_proxy
    );

    EXPORTED std::unique_ptr<IBackgroundServer> start_tcp_server_proxy(
        const Endpoint &local_endpoint,
        const Endpoint &udp_proxy
    );
}