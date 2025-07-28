#pragma once

#include "libtcp/client/ITcpClient.h"
#include "libtcp/ITcpSession.h"

#include <ace/Event_Handler.h>
#include <ace/Reactor.h>

class UdpTcpBinding final : public ACE_Event_Handler {
public:
    UdpTcpBinding(
        std::unique_ptr<tcp::ITcpClient> client,
        const std::shared_ptr<tcp::ITcpSession> &tcp_session)
        : client_(std::move(client)),
          tcp_session_(tcp_session) {
    }

    // ACE_Event_Handler interface
    int handle_input(ACE_HANDLE fd) override;

    [[nodiscard]] ACE_HANDLE get_handle() const override;

    // Registration methods
    void register_with_reactor(ACE_Reactor *reactor);
    void unregister_from_reactor();

    [[nodiscard]] tcp::ITcpSession& get_tcp_session() const { return *tcp_session_; }


private:
    // The client is stored too because it must outlive the tcp_session
    std::unique_ptr<tcp::ITcpClient> client_;
    std::shared_ptr<tcp::ITcpSession> tcp_session_;

    ACE_Reactor *reactor_ = nullptr;
    bool registered_ = false;
};
