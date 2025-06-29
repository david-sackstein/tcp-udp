#include "AceUdpClient.h"

#include <libacetools/ISocketIO.h>

#include <ace/Log_Msg.h>
#include <ace/INET_Addr.h>

#include <stdexcept>
#include <iostream>

AceUdpClient::AceUdpClient(const Endpoint& local_endpoint)
    : local_endpoint_(local_endpoint) {

    ACE_INET_Addr local_addr = to_ace_addr(local_endpoint_);

    if (socket_.open(local_addr) == -1) {
        throw std::runtime_error("Failed to open UDP socket on " + local_endpoint.to_string());
    }

    local_endpoint_ = get_bound_endpoint(socket_);

    printf("UdpClient: %s successfully bound\n", local_endpoint_.to_string().c_str());
}

AceUdpClient::~AceUdpClient() {
    if (socket_.get_handle() != ACE_INVALID_HANDLE) {
        socket_.close();
        printf("UdpClient: %s socket closed\n", local_endpoint_.to_string().c_str());
    }
}

bool AceUdpClient::send_to(const Endpoint& remote, ConstBuffer buffer) {
    ACE_INET_Addr remote_addr = to_ace_addr(remote);

    ssize_t sent = socket_.send(buffer.data, buffer.size, remote_addr);
    if (sent == -1 || static_cast<size_t>(sent) != buffer.size) {
        ACE_ERROR_RETURN((LM_ERROR, ACE_TEXT("Failed to send UDP data\n")), false);
    }

    printf("UdpClient: %s -> %s sent: \"%.*s\"\n",
           local_endpoint_.to_string().c_str(),
           remote.to_string().c_str(),
           (int)buffer.size,
           buffer.data);

    return true;
}

ssize_t AceUdpClient::receive_from(Buffer buffer, Endpoint& sender) {
    ACE_INET_Addr sender_addr;
    const ssize_t received = socket_.recv(buffer.data, buffer.size, sender_addr);
    if (received == -1) {
        ACE_ERROR_RETURN((LM_ERROR, ACE_TEXT("Failed to receive UDP data\n")), -1);
    }

    sender = to_endpoint(sender_addr);

    printf("UdpClient: %s <- %s received: \"%.*s\"\n",
           get_bound_endpoint(socket_).to_string().c_str(),
           sender.to_string().c_str(),
           (int)received,
           buffer.data);

    return received;
}