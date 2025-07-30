#include <libargsparser/Exports.h>
#include <liblogger/Exports.h>
#include <libudp/Exports.h>
#include <iostream>

int main(int argc, char* argv[]) {
    auto logger = logger::create_console_logger(logger::LogLevel::ERROR);

    const auto& local_addresses = udp::get_local_ipv4_addresses();

    std::string listen_endpoint_str = get_listen_endpoint(argc, argv, local_addresses);

    // Get --force flag value
    std::string force_flag = get_flag_value(argc, argv, "--force", "false");
    bool force = (force_flag == "true");

    // Port cleanup
    if (!check_and_cleanup_port(*logger, listen_endpoint_str, force)) {
        logger->log(logger::LogLevel::ERROR, "Port cleanup failed. Exiting.");
        return 1;
    }

    Endpoint listen_ep = Endpoint::from_string(listen_endpoint_str);

    std::unique_ptr<udp::IUdpClientHandler> handler = udp::create_udp_echo_handler(*logger);

    auto server = udp::start_udp_server(*logger, listen_ep, *handler);

    logger->log(logger::LogLevel::INFO, "UDP Server running on %s", server->get_local_endpoint().to_string().c_str());
    logger->log(logger::LogLevel::INFO, "Press Ctrl+C to stop the server...");

    server->stop();

    logger->log(logger::LogLevel::INFO, "Server stopped");

    return 0;
}
