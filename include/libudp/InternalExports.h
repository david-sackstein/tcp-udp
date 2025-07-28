#pragma once

// Internal exports that expose ACE types for internal components only

#include "Exports.h"

#include <ace/Reactor.h>

namespace udp {

    EXPORTED std::unique_ptr<IBlockingServer> create_udp_server(
        logger::ILogger &logger,
        const Endpoint &local_endpoint,
        IUdpClientHandler &handler,
        const std::shared_ptr<ACE_Reactor>& reactor);

    EXPORTED std::unique_ptr<IBackgroundServer> start_udp_server(
        logger::ILogger &logger,
        const Endpoint &local_endpoint,
        IUdpClientHandler &handler,
        const std::shared_ptr<ACE_Reactor> &reactor);

}