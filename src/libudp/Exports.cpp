#include "client/AceUdpClient.h"
#include "server/AceUdpServer.h"
#include "libudp/examples/UdpEchoHandler.h"

#include <background/BackgroundRunner.h>
#include <libacetools/ACEInitializer.h>

#include <libudp/InternalExports.h>

namespace udp {
    static std::vector<std::string> ip_addresses = ACEInitializer::init();

    EXPORTED const std::vector<std::string> &get_local_ipv4_addresses() {
        return ip_addresses;
    }

    EXPORTED std::unique_ptr<IUdpClient> create_udp_client(logger::ILogger &logger, const Endpoint &local_endpoint) {
        return std::make_unique<AceUdpClient>(logger, local_endpoint);
    }

    EXPORTED std::unique_ptr<IBlockingServer> create_udp_server(
        logger::ILogger &logger,
        const Endpoint &local_endpoint,
        IUdpClientHandler &handler) {
        return create_udp_server(logger, local_endpoint, handler, nullptr);
    }

    EXPORTED std::unique_ptr<IBlockingServer> create_udp_server(
        logger::ILogger &logger,
        const Endpoint &local_endpoint,
        IUdpClientHandler &handler,
        const std::shared_ptr<ACE_Reactor>& reactor) {
        return std::make_unique<AceUdpServer>(logger, local_endpoint, handler, reactor);
    }

    EXPORTED std::unique_ptr<IBackgroundServer> start_udp_server(
        logger::ILogger &logger,
        const Endpoint &local_endpoint,
        IUdpClientHandler &handler) {
        return start_udp_server(logger, local_endpoint, handler, nullptr);
    }

    EXPORTED std::unique_ptr<IBackgroundServer> start_udp_server(
     logger::ILogger &logger,
     const Endpoint &local_endpoint,
     IUdpClientHandler &handler,
     const std::shared_ptr<ACE_Reactor> &reactor) {

        return startServer(
            [reactor, &logger, &local_endpoint, &handler] {
                return create_udp_server(logger, local_endpoint, handler, reactor);
            },
            [](auto &server) {
                server.start();
            }
        );
    }


    EXPORTED std::unique_ptr<IUdpClientHandler> create_udp_echo_handler(logger::ILogger &logger) {
        return std::make_unique<UdpEchoHandler>(logger);
    }
}
