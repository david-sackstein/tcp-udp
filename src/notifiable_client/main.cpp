#include <libargsparser/Exports.h>
#include <liblogger/Exports.h>

#include "NotifiableClient.h"
#include "NotifiableClientArgs.h"

#include <thread>

int main(int argc, char* argv[]) {
    // Parse command line arguments first
    NotifiableClientArgs args;
    if (!args.parse(argc, argv)) {
        // Create a temporary logger for error reporting
        auto temp_logger = logger::create_console_logger(logger::LogLevel::ERROR);
        temp_logger->log(logger::LogLevel::ERROR, "Failed to parse arguments: %s", args.get_error_message().c_str());
        return 1;
    }

    // Check for --verbose flag for communication logger
    std::string verbose_value = get_flag_value(argc, argv, "--verbose", "false");
    bool verbose = (verbose_value == "true" || verbose_value == "1");

    // Create two loggers:
    // 1. Validation logger - always verbose (INFO level) for validation messages
    // 2. Communication logger - depends on verbose flag for communication logs
    auto validation_logger = logger::create_console_logger(logger::LogLevel::INFO);
    auto communication_logger =
        logger::create_console_logger(verbose ? logger::LogLevel::INFO : logger::LogLevel::ERROR);

    validation_logger->log(logger::LogLevel::INFO, "Notifiable Client");
    validation_logger->log(logger::LogLevel::INFO, "Client ID: %s", args.get_client_id().c_str());
    validation_logger->log(logger::LogLevel::INFO, "Request interval: %d ms", args.get_request_interval_ms());
    validation_logger->log(logger::LogLevel::INFO, "Connecting to server at: %s", args.get_endpoint().c_str());

    // Create and start client with two loggers
    NotifiableClient client(*validation_logger, *communication_logger);
    if (!client.connect(args.get_endpoint())) {
        return 1;
    }

    client.start(args);

    // Wait for user to stop (Ctrl+C or similar)
    validation_logger->log(logger::LogLevel::INFO, "Press Ctrl+C to stop...");
    std::this_thread::sleep_for(std::chrono::hours(24)); // Wait indefinitely

    client.stop();
    validation_logger->log(logger::LogLevel::INFO, "Notifiable client finished");
    return 0;
}