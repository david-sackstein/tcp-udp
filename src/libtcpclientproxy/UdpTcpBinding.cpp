#include "UdpTcpBinding.h"

#include <common/Constants.h>
#include <libacetools/Exports.h>
#include <libacetools/IOResultCode.h>

static ISocketIO& io_ = get_socket_io();

ACE_HANDLE UdpTcpBinding::get_handle() const {
    return tcp_session_->get_socket();
}

int UdpTcpBinding::handle_input(ACE_HANDLE) {
    auto result = tcp_session_->read(buffer_.view(), std::chrono::milliseconds(0));

    if (result.code == IOResultCode::Success && result.count > 0) {
        ConstBuffer tcp_data{buffer_.view().data, result.count};
        udp_session_.write_to(tcp_data, udp_sender_, common::STANDARD_TIMEOUT);
        return 0;
    }

    if (result.code == IOResultCode::ConnectionClosed || result.code == IOResultCode::Error) {
        unregister_from_reactor();
        return -1;
    }

    return 0;
}

void UdpTcpBinding::register_with_reactor(ACE_Reactor* reactor) {
    if (!registered_ && reactor) {
        reactor_ = reactor;
        if (reactor_->register_handler(this, ACE_Event_Handler::READ_MASK) == 0) {
            registered_ = true;
        }
    }
}

void UdpTcpBinding::unregister_from_reactor() {
    if (registered_ && reactor_) {
        reactor_->remove_handler(this, ACE_Event_Handler::READ_MASK);
        registered_ = false;
        reactor_ = nullptr;
    }
}