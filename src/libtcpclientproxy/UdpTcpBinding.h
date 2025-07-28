#pragma once

#include "libtcp/client/ITcpClient.h"
#include "libtcp/ITcpSession.h"

#include <common/OwnedBuffer.h>
#include <libudp/server/IUdpSession.h>
#include <ace/Event_Handler.h>
#include <ace/Reactor.h>

class UdpTcpBinding final : public ACE_Event_Handler {
public:
    UdpTcpBinding(
        std::unique_ptr<tcp::ITcpClient> client,
        const std::shared_ptr<tcp::ITcpSession> &tcp_session,
        udp::IUdpSession& udp_session,
        const Endpoint& udp_sender)
        : client_(std::move(client)),
          tcp_session_(tcp_session),
          udp_session_(udp_session),
          udp_sender_(udp_sender) {}

    ~UdpTcpBinding() override {
        unregister_from_reactor();
    }

    // ACE_Event_Handler interface
    int handle_input(ACE_HANDLE fd) override;

    [[nodiscard]] ACE_HANDLE get_handle() const override;

    // Registration methods
    void register_with_reactor(ACE_Reactor *reactor);
    void unregister_from_reactor();

    [[nodiscard]] tcp::ITcpSession& get_tcp_session() const { return *tcp_session_; }
    [[nodiscard]] const Endpoint& get_udp_sender() const { return udp_sender_; }

private:
    std::unique_ptr<tcp::ITcpClient> client_;
    std::shared_ptr<tcp::ITcpSession> tcp_session_;
    
    udp::IUdpSession& udp_session_;
    Endpoint udp_sender_;

    ACE_Reactor *reactor_ = nullptr;
    bool registered_ = false;

    OwnedBuffer buffer_{2048};
};
