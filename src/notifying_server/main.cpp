#include <libargsparser/Exports.h>
#include <liblogger/Exports.h>
#include <libtcp/Exports.h>
#include <iostream>

int main(int argc, char* argv[]) {
    auto logger = logger::create_console_logger(logger::LogLevel::ERROR);

    const auto& local_addresses = tcp::get_local_ipv4_addresses();

    std::string listen_endpoint_str = get_listen_endpoint(argc, argv, local_addresses);
    Endpoint listen_ep = Endpoint::from_string(listen_endpoint_str);

    std::unique_ptr<tcp::ITcpClientHandler> notification_handler = tcp::create_tcp_notification_handler(*logger);
    auto server = tcp::create_tcp_server(*logger, listen_ep, *notification_handler);

    logger->log(
        logger::LogLevel::INFO, "Notifying Server running on %s", server->get_local_endpoint().to_string().c_str());
    logger->log(logger::LogLevel::INFO, "Press Ctrl+C to stop the server...");

    server->start();
    server->stop();

    logger->log(logger::LogLevel::INFO, "Notifying server shut down");
    return 0;
}