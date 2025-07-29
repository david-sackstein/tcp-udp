#pragma once

#include <common/EndpointPair.h>
#include <liblogger/ILogger.h>
#include <libtcp/ITcpSession.h>

#include <ace/INET_Addr.h>
#include <ace/SOCK_Stream.h>

class AceTcpClientSession final : public tcp::ITcpSession {
public:
    AceTcpClientSession(logger::ILogger& logger,
        const Endpoint& local_endpoint,
        const ACE_SOCK_Stream& client_socket,
        const ACE_INET_Addr& client_address);

    ~AceTcpClientSession() override;

    [[nodiscard]] Endpoint get_peer() const override;
    [[nodiscard]] int get_socket() const override {
        return client_sock_.get_handle();
    }

    IOResult read(Buffer buffer, std::chrono::milliseconds timeout) override;

    IOResult write(ConstBuffer buffer, std::chrono::milliseconds timeout) override;

    void close() override;

private:
    logger::ILogger& logger_;
    EndpointPair endpoint_pair_;
    ACE_SOCK_Stream client_sock_;
    ACE_INET_Addr client_addr_;
};
