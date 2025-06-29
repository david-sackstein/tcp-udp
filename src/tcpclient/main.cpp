#include <libtcp/Exports.h>

#include <iostream>
#include "common/OwnedBuffer.h"

static std::chrono::milliseconds block = std::chrono::milliseconds::max();

int main() {
    std::cout << "TCP Echo Client" << std::endl;

    auto client = tcp::create_tcp_client();
    if (!client) {
        std::cerr << "Failed to create client" << std::endl;
        return 1;
    }

    const Endpoint endpoint = Endpoint::loop_back(12345);

    auto session = client->connect(Endpoint::any_loop_back(), endpoint);
    if (!session) {
        std::cerr << "Failed to connect to server" << std::endl;
        return 1;
    }

    const std::string message = "hello";

    std::cout << "\n=== Echo Test ===" << std::endl;

    if (IOResult result = session->write(ConstBuffer(message.data(), message.size()), block);
        result.code != IOResultCode::Success || result.count != message.size()) {
        std::cerr << "Failed to send message" << std::endl;
        return 1;
    }

    // Read response
    OwnedBuffer buffer_in(1024);

    const auto result = session->read(buffer_in.view(), block);
    if (result.code != IOResultCode::Success) {
        std::cerr << "Failed to receive response" << std::endl;
        return 1;
    }
    std::string response(buffer_in.view().data, result.count);

    // Verify echo with "echo" prefix
    std::string expected_response = "echo" + message;
    if (response == expected_response) {
        std::cout << "\n✓ Echo test successful! Server correctly echoed: " << response << std::endl;
    } else {
        std::cout << "\n✗ Echo test failed! Expected: " << expected_response << ", Got: " << response << std::endl;
    }

    // Disconnect
    client->disconnect();

    std::cout << "\nClient finished successfully" << std::endl;
    return 0;
}
