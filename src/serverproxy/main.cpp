#include <libargsparser/Exports.h>
#include <liblogger/Exports.h>
#include <libtcp/Exports.h>
#include <iostream>
#include <vector>

int main(int argc, char* argv[]) {
    auto logger = logger::create_console_logger(logger::LogLevel::ERROR);

    const auto& local_addresses = tcp::get_local_ipv4_addresses();

    auto endpoint_strings = get_endpoint_configuration(argc, argv, local_addresses);
    std::string listen_endpoint_str = endpoint_strings.first;

    // Port cleanup
    if (!check_and_cleanup_port(*logger, listen_endpoint_str)) {
        logger->log(logger::LogLevel::ERROR, "Port cleanup failed. Exiting.");
        return 1;
    }

    Endpoint listen_ep = Endpoint::from_string(endpoint_strings.first);
    Endpoint forward_ep = Endpoint::from_string(endpoint_strings.second);

    logger->log(logger::LogLevel::INFO, "Server Proxy running on %s", listen_ep.to_string().c_str());
    logger->log(
        logger::LogLevel::INFO, "Forwarding connections to TCP Server Proxy at %s", forward_ep.to_string().c_str());
    logger->log(logger::LogLevel::INFO, "Press Ctrl+C to stop the server...");

    // Create a simple TCP server that forwards to the TCP Server Proxy
    auto server =
        tcp::create_tcp_server(logger, listen_ep, [&logger, forward_ep](tcp::ITcpClientHandler& client_handler) {
            // Create a client connection to the TCP Server Proxy
            auto client = tcp::create_tcp_client(logger, forward_ep);
            if (client->connect()) {
                logger->log(logger::LogLevel::INFO, "Server Proxy: Connected to TCP Server Proxy");

                // Forward data between client and TCP Server Proxy
                // This is a simplified implementation
                std::string buffer;
                while (client_handler.receive(buffer)) {
                    client->send(buffer);
                    std::string response;
                    if (client->receive(response)) {
                        client_handler.send(response);
                    }
                }
            } else {
                logger->log(logger::LogLevel::ERROR, "Server Proxy: Failed to connect to TCP Server Proxy");
            }
        });

    server->start();
    server->stop();

    logger->log(logger::LogLevel::INFO, "serverproxy shut down");

    return 0;
}