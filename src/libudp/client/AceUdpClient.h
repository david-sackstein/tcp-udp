#pragma once

#include <libudp/client/IUdpClient.h>
#include <common/Buffer.h>

#include <ace/SOCK_Dgram.h>
#include <ace/INET_Addr.h>

#include <string>

class AceUdpClient final : public udp::IUdpClient {
public:
    explicit AceUdpClient(const Endpoint& local_endpoint);
    ~AceUdpClient() override;

    [[nodiscard]] const Endpoint& get_local_endpoint() const override {
        return local_endpoint_;
    }

    bool send_to(const Endpoint& remote, ConstBuffer buffer) override;
    ssize_t receive_from(Buffer buffer, Endpoint& sender) override;

private:
    Endpoint local_endpoint_;
    ACE_SOCK_Dgram socket_;
};
