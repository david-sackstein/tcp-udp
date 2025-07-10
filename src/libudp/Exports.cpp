#include "client/AceUdpClient.h"
#include "server/AceUdpServer.h"
#include "libudp/examples/UdpEchoHandler.h"

#include <background/BackgroundRunner.h>
#include <libacetools/ACEInitializer.h>

namespace udp {
    static std::vector<std::string> ip_addresses = ACEInitializer::init();

    EXPORTED const std::vector<std::string>& get_local_ipv4_addresses(){
        return ip_addresses;
    }

    EXPORTED std::unique_ptr<IUdpClient> create_udp_client(const Endpoint& local_endpoint) {
        return std::make_unique<AceUdpClient>(local_endpoint);
    }

    EXPORTED std::unique_ptr<IBlockingServer> create_udp_server(const Endpoint& local_endpoint, IUdpClientHandler &handler) {
        return std::make_unique<AceUdpServer>(local_endpoint, handler);
    }

    EXPORTED std::unique_ptr<IBackgroundServer> start_udp_server(const Endpoint& local_endpoint, IUdpClientHandler &handler) {
        return startServer(
                [&] { return create_udp_server(local_endpoint, handler); },
                [](auto &server) { server.start(); }
        );
    }

    EXPORTED std::unique_ptr<IUdpClientHandler> create_udp_echo_handler() {
        return std::make_unique<UdpEchoHandler>();
    }
}
