#pragma once

#include "AceUdpClientSession.h"

#include <libudp/server/IUdpClientHandler.h>
#include <common/server/IBlockingServer.h>
#include <common/task/ITask.h>
#include <libacetools/ISignalRegistration.h>
#include <liblogger/ILogger.h>

#include <ace/Reactor.h>
#include <ace/SOCK_Dgram.h>
#include <ace/Event_Handler.h>

#include <memory>
#include <vector>

class AceUdpServer final : public IBlockingServer, public ACE_Event_Handler {
public:
    AceUdpServer(logger::ILogger& logger, const Endpoint& local_endpoint, udp::IUdpClientHandler &handler);

    ~AceUdpServer() override;

    [[nodiscard]] const Endpoint& get_local_endpoint() const override;

    void start() override;

    void stop() override;

    int handle_input(ACE_HANDLE fd) override;

    [[nodiscard]] ACE_HANDLE get_handle() const override;

private:
    void stop_all_tasks();

    logger::ILogger& logger_;
    ACE_Reactor reactor_;
    ACE_SOCK_Dgram socket_;
    udp::IUdpClientHandler &handler_;
    std::unique_ptr<ISignalRegistration> signal_registration_;
    Endpoint local_endpoint_;

    bool stopped_ = false;
    std::vector<std::unique_ptr<ITask>> tasks_;
    std::atomic<bool> is_cancelled_{false};
    std::unique_ptr<AceUdpClientSession> session_;
};

