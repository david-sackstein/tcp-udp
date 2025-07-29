#pragma once

#include <common/OwnedBuffer.h>
#include <liblogger/ILogger.h>
#include <libudp/client/IUdpClient.h>

#include <memory>
#include <unordered_map>

// Forward declarations
namespace tcp {
class ITcpSession;
}

class ServerProxyUdpHandler final {
public:
    explicit ServerProxyUdpHandler(logger::ILogger& logger);

    // Handle UDP input from client proxy responses
    int handle_udp_input(udp::IUdpClient& udp_client);

    // Register TCP session for a specific client endpoint
    void register_tcp_session(const std::string& client_key, std::shared_ptr<tcp::ITcpSession> tcp_session);

    // Unregister TCP session
    void unregister_tcp_session(const std::string& client_key);

private:
    logger::ILogger& logger_;
    std::unordered_map<std::string, std::shared_ptr<tcp::ITcpSession>> tcp_sessions_;
    OwnedBuffer buffer_{2048};
};