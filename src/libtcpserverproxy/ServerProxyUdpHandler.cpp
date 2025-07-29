#include "ServerProxyUdpHandler.h"

#include <common/Constants.h>
#include <common/StringUtils.h>
#include <libacetools/IOResultCode.h>
#include <libtcp/ITcpSession.h>

#include <stdexcept>

std::chrono::milliseconds write_timeout = common::STANDARD_TIMEOUT;

ServerProxyUdpHandler::ServerProxyUdpHandler(logger::ILogger& logger) : logger_(logger) {}

int ServerProxyUdpHandler::handle_udp_input(udp::IUdpClient& udp_client) {
    try {
        Endpoint udp_sender;
        ssize_t received = udp_client.receive_from(buffer_.view(), udp_sender);

        if (received == -1) {
            logger_.log(logger::LogLevel::ERROR, "ServerProxyUdpHandler: Failed to receive UDP data");
            return -1;
        }

        if (received == 0) {
            return 0;
        }

        ConstBuffer udp_data{buffer_.view().data, static_cast<size_t>(received)};
        logger_.log(logger::LogLevel::INFO, "ServerProxyUdpHandler: Received UDP response from %s: '%s'",
            udp_sender.to_string().c_str(), std::string(udp_data.data, udp_data.size).c_str());

        // Forward to all registered TCP sessions
        for (auto& [client_key, tcp_session] : tcp_sessions_) {
            if (tcp_session) {
                auto write_result = tcp_session->write(udp_data, write_timeout);
                if (write_result.code == IOResultCode::Success) {
                    logger_.log(logger::LogLevel::INFO,
                        "ServerProxyUdpHandler: Forwarded UDP response to TCP client %s", client_key.c_str());
                } else {
                    logger_.log(logger::LogLevel::ERROR,
                        "ServerProxyUdpHandler: Failed to forward to TCP client %s: %s", client_key.c_str(),
                        write_result.error_message.c_str());
                }
            }
        }

        return 0;
    } catch (std::runtime_error& e) {
        logger_.log(logger::LogLevel::ERROR, "ServerProxyUdpHandler: Exception: %s", e.what());
        return -1;
    }
}

void ServerProxyUdpHandler::register_tcp_session(const std::string& client_key,
    std::shared_ptr<tcp::ITcpSession> tcp_session) {
    tcp_sessions_[client_key] = tcp_session;
    logger_.log(
        logger::LogLevel::INFO, "ServerProxyUdpHandler: Registered TCP session for client %s", client_key.c_str());
}

void ServerProxyUdpHandler::unregister_tcp_session(const std::string& client_key) {
    tcp_sessions_.erase(client_key);
    logger_.log(
        logger::LogLevel::INFO, "ServerProxyUdpHandler: Unregistered TCP session for client %s", client_key.c_str());
}