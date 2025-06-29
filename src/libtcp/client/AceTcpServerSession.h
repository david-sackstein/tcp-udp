#pragma once

#include <libtcp/ITcpSession.h>
#include <common/EndpointPair.h>

#include <ace/SOCK_Stream.h>
#include <ace/INET_Addr.h>

class AceTcpServerSession final : public tcp::ITcpSession {
public:
    AceTcpServerSession(
        EndpointPair endpoint_pair,
        const ACE_SOCK_Stream &stream,
        const ACE_INET_Addr &addr);

    ~AceTcpServerSession() override;

    [[nodiscard]] Endpoint get_peer() const override;

    IOResult read(Buffer buffer, std::chrono::milliseconds timeout) override;

    IOResult write(ConstBuffer buffer, std::chrono::milliseconds timeout) override;

    void close() override;

private:
    EndpointPair endpoint_pair_;
    ACE_SOCK_Stream socket_;
    ACE_INET_Addr server_addr_;
};
