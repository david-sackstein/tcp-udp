#include <liblogger/Exports.h>
#include <libtcp/Exports.h>

#include <common/Constants.h>
#include "common/OwnedBuffer.h"

#include <iostream>

static std::chrono::milliseconds block = std::chrono::milliseconds::max();

int main() {
    auto logger = logger::create_console_logger(logger::LogLevel::ERROR);
    logger->log(logger::LogLevel::INFO, "TCP Echo Client");

    auto client = tcp::create_tcp_client(*logger);
    if (!client) {
        logger->log(logger::LogLevel::ERROR, "Failed to create client");
        return 1;
    }

    const Endpoint endpoint = Endpoint::loop_back(common::DEFAULT_TCP_PORT);

    auto session = client->connect(Endpoint::any_loop_back(), endpoint);
    if (!session) {
        logger->log(logger::LogLevel::ERROR, "Failed to connect to server");
        return 1;
    }

    const std::string message = "hello";

    logger->log(logger::LogLevel::INFO, "=== Echo Test ===");

    if (IOResult result = session->write(ConstBuffer(message.data(), message.size()), block);
        result.code != IOResultCode::Success || result.count != message.size()) {
        logger->log(logger::LogLevel::ERROR, "Failed to send message: %s", result.error_message.c_str());
        return 1;
    }

    OwnedBuffer buffer_in(common::STANDARD_BUFFER_SIZE);

    const auto result = session->read(buffer_in.view(), block);
    if (result.code != IOResultCode::Success) {
        logger->log(logger::LogLevel::ERROR, "Failed to receive response: %s", result.error_message.c_str());
        return 1;
    }
    std::string response(buffer_in.view().data, result.count);

    std::string expected_response = "echo" + message;
    if (response == expected_response) {
        logger->log(logger::LogLevel::INFO, "✓ Echo test successful! Server correctly echoed: %s", response.c_str());
    } else {
        logger->log(logger::LogLevel::ERROR, "✗ Echo test failed! Expected: %s, Got: %s", expected_response.c_str(),
            response.c_str());
    }

    client->disconnect();

    logger->log(logger::LogLevel::INFO, "Client finished successfully");
    return 0;
}
