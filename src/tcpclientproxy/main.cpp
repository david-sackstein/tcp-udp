#include <libargsparser/Exports.h>
#include <liblogger/Exports.h>
#include <libtcpclientproxy/Exports.h>
#include <iostream>

int main(int argc, char* argv[]) {
    auto logger = logger::create_console_logger(logger::LogLevel::ERROR);

    const auto& local_addresses = client_proxy::get_local_ipv4_addresses();

    auto endpoint_strings = get_endpoint_configuration(argc, argv, local_addresses);
    std::string listen_endpoint_str = endpoint_strings.first;

    // Get --force flag value
    std::string force_flag = get_flag_value(argc, argv, "--force", "false");
    bool force = (force_flag == "true");

    // Port cleanup
    if (!check_and_cleanup_port(*logger, listen_endpoint_str, force)) {
        logger->log(logger::LogLevel::ERROR, "Port cleanup failed. Exiting.");
        return 1;
    }

    Endpoint listen_ep = Endpoint::from_string(endpoint_strings.first);
    Endpoint forward_ep = Endpoint::from_string(endpoint_strings.second);

    logger->log(logger::LogLevel::INFO, "TCP Client Proxy running on %s", listen_ep.to_string().c_str());
    logger->log(logger::LogLevel::INFO, "Forwarding connections to TCP server at %s", forward_ep.to_string().c_str());
    logger->log(logger::LogLevel::INFO, "Press Ctrl+C to stop the server...");

    auto server = client_proxy::create_tcp_client_proxy(*logger, listen_ep, forward_ep);

    server->start();
    server->stop();

    logger->log(logger::LogLevel::INFO, "tcpclientproxy shut down");

    return 0;
}
