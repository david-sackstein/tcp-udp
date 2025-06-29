#include "AceUdpClientSession.h"

#include <libacetools/Exports.h>

static ISocketIO& io_ = get_socket_io();

AceUdpClientSession::AceUdpClientSession(
    Endpoint  local_endpoint,
    ACE_SOCK_Dgram& server_socket)
    : local_endpoint_(std::move(local_endpoint)),
      server_socket_(server_socket) {}

IOResult AceUdpClientSession::read_from(Buffer buffer, Endpoint& sender, std::chrono::milliseconds timeout) {
    ACE_INET_Addr sender_addr;
    IOResult result = io_.read_from(server_socket_, buffer, sender_addr, timeout);

    if (result.code == IOResultCode::Success && result.count > 0) {
        sender = to_endpoint(sender_addr);
        printf("UdpClientSession: <- %s received: '%s'\n",
               sender.to_string().c_str(),
               std::string(buffer.data, result.count).c_str());
    }

    return result;
}

IOResult AceUdpClientSession::write_to(ConstBuffer buffer, const Endpoint& remote, std::chrono::milliseconds timeout) {
    ACE_INET_Addr dest_addr = to_ace_addr(remote);
    IOResult result = io_.write(server_socket_, buffer, dest_addr, timeout);

    if (result.code == IOResultCode::Success && result.count > 0) {
        printf("UdpClientSession: -> %s sent: '%s'\n",
               remote.to_string().c_str(),
               std::string(buffer.data, result.count).c_str());
    }

    return result;
} 