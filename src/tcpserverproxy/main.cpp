#include <libargsparser/Exports.h>
#include <liblogger/Exports.h>
#include <libtcpserverproxy/Exports.h>
#include <iostream>

int main(int argc, char* argv[]) {
    std::string verbose_value = get_flag_value(argc, argv, "--verbose", "false");
    bool verbose = (verbose_value == "true" || verbose_value == "1");

    auto logger = logger::create_console_logger(verbose ? logger::LogLevel::INFO : logger::LogLevel::ERROR);

    const auto& local_addresses = server_proxy::get_local_ipv4_addresses();

    auto endpoint_strings = get_endpoint_configuration(argc, argv, local_addresses);
    std::string listen_endpoint_str = endpoint_strings.first;

    if (!check_and_cleanup_port(*logger, listen_endpoint_str)) {
        logger->log(logger::LogLevel::ERROR, "Port cleanup failed. Exiting.");
        return 1;
    }

    Endpoint listen_ep = Endpoint::from_string(endpoint_strings.first);
    Endpoint forward_ep = Endpoint::from_string(endpoint_strings.second);

    logger->log(logger::LogLevel::INFO, "TCP Server Proxy running on %s", listen_ep.to_string().c_str());
    logger->log(logger::LogLevel::INFO, "Forwarding connections to UDP proxy at %s", forward_ep.to_string().c_str());
    logger->log(logger::LogLevel::INFO, "Press Ctrl+C to stop the server...");

    auto server = server_proxy::create_tcp_server_proxy(*logger, listen_ep, forward_ep);

    server->start();
    server->stop();

    logger->log(logger::LogLevel::INFO, "tcpserverproxy shut down");

    return 0;
}
