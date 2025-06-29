#include "AceTcpClient.h"

#include <libacetools/Exports.h>
#include <ace/Log_Msg.h>

#include <iostream>

AceTcpClient::~AceTcpClient() noexcept {
    disconnect();
}

std::shared_ptr<tcp::ITcpSession> AceTcpClient::connect(const Endpoint &local, const Endpoint &remote) {

    ACE_INET_Addr local_addr = to_ace_addr(local);
    ACE_INET_Addr remote_addr = to_ace_addr(remote);

    ACE_SOCK_Stream socket;

    if (connector_.connect(socket, remote_addr, nullptr, local_addr) == -1) {
        ACE_ERROR((LM_ERROR, ACE_TEXT("connect failed: %s:%d -> %s:%d (errno=%d)\n"),
            local_addr.get_host_addr(), local_addr.get_port_number(),
            remote_addr.get_host_addr(), remote_addr.get_port_number(),
            ACE_OS::last_error()));
        return {};
    }

    // Set linger timeout to prevent socket lingering
    get_socket_io().set_linger_timeout(socket);

    endpoint_pair_ = {get_bound_endpoint(socket), remote};

    printf("TcpClient: %s -> %s connected\n",
           endpoint_pair_.local.to_string().c_str(),
           endpoint_pair_.remote.to_string().c_str());

    session_ = std::make_shared<AceTcpServerSession>(
        endpoint_pair_, std::move(socket), remote_addr);

    return session_;
}

void AceTcpClient::disconnect() {
    if (session_) {
        session_->close();
        session_.reset();

        printf("TcpClient: %s -> %s disconnected\n",
               endpoint_pair_.local.to_string().c_str(),
               endpoint_pair_.remote.to_string().c_str());
    }
}

