#include <liblogger/Exports.h>

#include "NotifiableClient.h"
#include "NotifiableClientArgs.h"

#include <thread>

int main(int argc, char* argv[]) {
    auto logger = logger::create_console_logger(logger::LogLevel::ERROR);
    logger->log(logger::LogLevel::INFO, "Notifiable Client");

    // Parse command line arguments
    NotifiableClientArgs args;
    if (!args.parse(argc, argv)) {
        logger->log(logger::LogLevel::ERROR, "Failed to parse arguments: %s", args.get_error_message().c_str());
        return 1;
    }

    logger->log(logger::LogLevel::INFO, "Client ID: %s", args.get_client_id().c_str());
    logger->log(logger::LogLevel::INFO, "Request interval: %d ms", args.get_request_interval_ms());
    logger->log(logger::LogLevel::INFO, "Connecting to server at: %s", args.get_endpoint().c_str());

    // Create and start client
    NotifiableClient client(*logger);
    if (!client.connect(args.get_endpoint())) {
        return 1;
    }

    client.start(args);

    // Wait for user to stop (Ctrl+C or similar)
    logger->log(logger::LogLevel::INFO, "Press Ctrl+C to stop...");
    std::this_thread::sleep_for(std::chrono::hours(24)); // Wait indefinitely

    client.stop();
    logger->log(logger::LogLevel::INFO, "Notifiable client finished");
    return 0;
}