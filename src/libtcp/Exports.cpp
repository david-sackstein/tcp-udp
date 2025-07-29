#include "server/AceTcpServer.h"
#include "client/AceTcpClient.h"
#include "examples/TcpEchoHandler.h"

#include <background/BackgroundRunner.h>
#include <libacetools/ACEInitializer.h>

#include <libtcp/Exports.h>

namespace tcp {
    static std::vector<std::string> ip_addresses = ACEInitializer::init();

    EXPORTED const std::vector<std::string> &get_local_ipv4_addresses() {
        return ip_addresses;
    }

    EXPORTED std::unique_ptr<ITcpClient> create_tcp_client(logger::ILogger &logger) {
        return std::make_unique<AceTcpClient>(logger);
    }

    EXPORTED std::unique_ptr<IBlockingServer> create_tcp_server(
        logger::ILogger &logger, const Endpoint &local_endpoint, ITcpClientHandler &handler) {
        return std::make_unique<AceTcpServer>(logger, local_endpoint, handler);
    }

    EXPORTED std::unique_ptr<IBlockingServer> create_tcp_server(
        logger::ILogger &logger, const Endpoint &local_endpoint, ITcpClientHandler &handler,
        const std::shared_ptr<ACE_Reactor>& reactor) {
        return std::make_unique<AceTcpServer>(logger, local_endpoint, handler, reactor);
    }

    EXPORTED std::unique_ptr<IBackgroundServer> start_tcp_server(
        logger::ILogger &logger,
        const Endpoint &local_endpoint,
        ITcpClientHandler &handler) {
        return startServer(
                [&] { return create_tcp_server(logger, local_endpoint, handler); },
                [](auto &server) { server.start(); }
        );
    }

    EXPORTED std::unique_ptr<ITcpClientHandler> create_tcp_echo_handler(logger::ILogger& logger) {
        return std::make_unique<TcpEchoHandler>(logger);
    }
}
