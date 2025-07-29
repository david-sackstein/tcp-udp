#pragma once

#include <libudp/client/IUdpClient.h>
#include <common/Buffer.h>
#include <liblogger/ILogger.h>

#include <ace/SOCK_Dgram.h>
#include <ace/INET_Addr.h>

#include <string>

class AceUdpClient final : public udp::IUdpClient {
public:
    AceUdpClient(logger::ILogger& logger, const Endpoint& local_endpoint);
    ~AceUdpClient() override;

    [[nodiscard]] const Endpoint& get_local_endpoint() const override {
        return local_endpoint_;
    }

    bool send_to(const Endpoint& remote, ConstBuffer buffer) override;
    ssize_t receive_from(Buffer buffer, Endpoint& sender) override;

    // For reactor integration
    [[nodiscard]] ACE_HANDLE get_socket() const { return socket_.get_handle(); }

private:
    Endpoint local_endpoint_;
    ACE_SOCK_Dgram socket_;
    logger::ILogger& logger_;
};
