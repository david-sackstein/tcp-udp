#pragma once

#include "AceTcpServerSession.h"

#include <liblogger/ILogger.h>
#include "libtcp/client/ITcpClient.h"

#include <ace/SOCK_Connector.h>

class AceTcpClient final : public tcp::ITcpClient {
public:
    explicit AceTcpClient(logger::ILogger& logger);
    ~AceTcpClient() noexcept override;

    std::shared_ptr<tcp::ITcpSession> connect(const Endpoint& local, const Endpoint& remote) override;
    void disconnect() override;

private:
    ACE_SOCK_Connector connector_;
    std::shared_ptr<AceTcpServerSession> session_;
    logger::ILogger& logger_;

    EndpointPair endpoint_pair_;
};
