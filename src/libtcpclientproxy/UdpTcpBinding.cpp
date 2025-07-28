#include "UdpTcpBinding.h"

ACE_HANDLE UdpTcpBinding::get_handle() const {
    return tcp_session_->get_socket();
}

int UdpTcpBinding::handle_input(ACE_HANDLE) {
    // For now, just return 0 - no actual TCP reading yet
    // This will be implemented in Step 4
    return 0;
}

void UdpTcpBinding::register_with_reactor(ACE_Reactor* reactor) {
    if (!registered_ && reactor) {
        reactor_ = reactor;
        reactor_->register_handler(this, READ_MASK);
        registered_ = true;
    }
}

void UdpTcpBinding::unregister_from_reactor() {
    if (registered_ && reactor_) {
        reactor_->remove_handler(this, READ_MASK);
        registered_ = false;
        reactor_ = nullptr;
    }
} 