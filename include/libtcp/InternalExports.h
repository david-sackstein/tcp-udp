#pragma once

// Internal exports that expose ACE types for internal components only

#include "Exports.h"

#include <ace/Reactor.h>

namespace tcp {

    EXPORTED std::unique_ptr<IBlockingServer> create_tcp_server(
        logger::ILogger &logger,
        const Endpoint &local_endpoint,
        ITcpClientHandler &handler,
        const std::shared_ptr<ACE_Reactor>& reactor);

} 