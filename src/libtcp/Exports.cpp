#include "client/AceTcpClient.h"
#include "server/AceTcpServer.h"
#include "examples/TcpEchoHandler.h"

#include <libtcp/Exports.h>
#include <libacetools/ACEInitializer.h>

#include <background/BackgroundRunner.h>

namespace tcp {

    static std::vector<std::string> ip_addresses = ACEInitializer::init();

    EXPORTED const std::vector<std::string>& get_local_ipv4_addresses(){
        return ip_addresses;
    }

    EXPORTED std::unique_ptr<ITcpClient> create_tcp_client() {
        return std::make_unique<AceTcpClient>();
    }

    EXPORTED std::unique_ptr<IBlockingServer> create_tcp_server(
        const Endpoint& local_endpoint, ITcpClientHandler &handler) {
        return std::make_unique<AceTcpServer>(local_endpoint, handler);
    }

    EXPORTED std::unique_ptr<IBackgroundServer> start_tcp_server(
        const Endpoint& local_endpoint, ITcpClientHandler &handler) {
        return startServer(
                [&] { return create_tcp_server(local_endpoint, handler); },
                [](auto &server) { server.start(); }
        );
    }

    EXPORTED std::unique_ptr<ITcpClientHandler> create_tcp_echo_handler() {
        return std::make_unique<TcpEchoHandler>();
    }
}
