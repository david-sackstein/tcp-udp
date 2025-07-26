#include "AceTcpClientSession.h"

#include <libacetools/Exports.h>
#include <libacetools/IOResultCode.h>

#include <ace/Time_Value.h>

static ISocketIO& io_ = get_socket_io();

AceTcpClientSession::AceTcpClientSession(
    const Endpoint& local_endpoint,
    const ACE_SOCK_Stream &client_socket,
    const ACE_INET_Addr &client_address)
    : endpoint_pair_ {local_endpoint, to_endpoint(client_address)},
      client_sock_(client_socket),
      client_addr_(client_address) {}

AceTcpClientSession::~AceTcpClientSession() {
    close();
}

Endpoint AceTcpClientSession::get_peer() const {
    return to_endpoint(client_addr_);
}

IOResult AceTcpClientSession::read(Buffer buffer, std::chrono::milliseconds timeout) {
    IOResult result = io_.read(client_sock_, buffer, timeout);

    if (result.code == IOResultCode::Success && result.count > 0) {
        printf("TcpClientSession: %s <- %s received: '%s'\n",
               endpoint_pair_.local.to_string().c_str(),
               endpoint_pair_.remote.to_string().c_str(),
               std::string(buffer.data, result.count).c_str());
    }

    return result;
}

IOResult AceTcpClientSession::write(ConstBuffer buffer, std::chrono::milliseconds timeout) {

    IOResult result = io_.write(client_sock_, buffer, timeout);

    if (result.code == IOResultCode::Success && result.count > 0) {
        printf("TcpClientSession: %s -> %s sent: '%s'\n",
               endpoint_pair_.local.to_string().c_str(),
               endpoint_pair_.remote.to_string().c_str(),
               std::string(buffer.data, result.count).c_str());
    }

    return result;
}

void AceTcpClientSession::close() {
    if (client_sock_.get_handle() != ACE_INVALID_HANDLE) {
        client_sock_.close();
    }
}