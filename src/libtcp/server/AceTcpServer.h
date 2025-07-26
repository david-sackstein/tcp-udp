#pragma once

#include "AceTcpServerAcceptor.h"

#include <common/server/IBlockingServer.h>
#include <libtcp/server/ITcpClientHandler.h>
#include <libacetools/ISignalRegistration.h>
#include <liblogger/ILogger.h>

#include <ace/Reactor.h>

class AceTcpServer final : public IBlockingServer {
public:
    AceTcpServer(logger::ILogger& logger, const Endpoint& local_endpoint, tcp::ITcpClientHandler& handler);

    [[nodiscard]] const Endpoint& get_local_endpoint() const override { return acceptor_.get_local_endpoint(); }

    void start() override;
    void stop() override;

private:
    logger::ILogger& logger_;
    tcp::ITcpClientHandler& handler_;
    ACE_Reactor reactor_;
    AceTcpServerAcceptor acceptor_;
    std::unique_ptr<ISignalRegistration> signal_registration_;
    Endpoint local_endpoint_;
};
