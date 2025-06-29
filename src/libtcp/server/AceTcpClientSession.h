#pragma once

#include <common/EndpointPair.h>
#include <libtcp/ITcpSession.h>

#include <ace/SOCK_Stream.h>
#include <ace/INET_Addr.h>

#include <string>
#include <atomic>

class AceTcpClientSession final : public tcp::ITcpSession {
public:
    AceTcpClientSession(
        const Endpoint& local_endpoint,
        const ACE_SOCK_Stream &client_socket,
        const ACE_INET_Addr &client_address);

    ~AceTcpClientSession() override;

    [[nodiscard]] Endpoint get_peer() const override;

    IOResult read(Buffer buffer, std::chrono::milliseconds timeout) override;

    IOResult write(ConstBuffer buffer, std::chrono::milliseconds timeout) override;

    void close() override;

private:
    EndpointPair endpoint_pair_;

    ACE_SOCK_Stream client_sock_;
    ACE_INET_Addr client_addr_;
};
