#pragma once

#include <common/Exported.h>
#include <common/Endpoint.h>
#include "common/server/IBackgroundServer.h"
#include "common/server/IBlockingServer.h"

#include <vector>
#include <memory>

namespace client_proxy {
    EXPORTED const std::vector<std::string>& get_local_ipv4_addresses();

    EXPORTED std::unique_ptr<IBlockingServer> create_tcp_client_proxy(
            const Endpoint &local_endpoint,
            const Endpoint &tcp_server);

    EXPORTED std::unique_ptr<IBackgroundServer> start_tcp_client_proxy(
            const Endpoint &local_endpoint,
            const Endpoint &tcp_server);
}