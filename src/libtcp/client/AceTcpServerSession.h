#pragma once

#include <common/EndpointPair.h>
#include <liblogger/ILogger.h>
#include <libtcp/ITcpSession.h>

#include <ace/INET_Addr.h>
#include <ace/SOCK_Stream.h>

class AceTcpServerSession final : public tcp::ITcpSession {
public:
    AceTcpServerSession(logger::ILogger& logger,
        EndpointPair endpoint_pair,
        const ACE_SOCK_Stream& stream,
        const ACE_INET_Addr& addr);

    ~AceTcpServerSession() override;

    [[nodiscard]] Endpoint get_peer() const override;
    [[nodiscard]] int get_socket() const override {
        return socket_.get_handle();
    }

    IOResult read(Buffer buffer, std::chrono::milliseconds timeout) override;

    IOResult write(ConstBuffer buffer, std::chrono::milliseconds timeout) override;

    void close() override;

private:
    logger::ILogger& logger_;
    EndpointPair endpoint_pair_;
    ACE_SOCK_Stream socket_;
    ACE_INET_Addr server_addr_;
};
