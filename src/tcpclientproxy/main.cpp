#include <libargsparser/Exports.h>
#include <liblogger/Exports.h>
#include <libtcpclientproxy/Exports.h>
#include <iostream>

int main(int argc, char* argv[]) {
    std::string verbose_value = get_flag_value(argc, argv, "--verbose", "false");
    bool verbose = (verbose_value == "true" || verbose_value == "1");

    auto logger = logger::create_console_logger(verbose ? logger::LogLevel::INFO : logger::LogLevel::ERROR);

    const auto& local_addresses = client_proxy::get_local_ipv4_addresses();

    auto endpoint_strings = get_endpoint_configuration(argc, argv, local_addresses);
    std::string listen_endpoint_str = endpoint_strings.first;

    if (!check_and_cleanup_port(*logger, listen_endpoint_str)) {
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
