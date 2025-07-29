#include "AceTcpServerSession.h"

#include <libacetools/IOResultCode.h>
#include <libacetools/Exports.h>

static ISocketIO& io_ = get_socket_io();

AceTcpServerSession::AceTcpServerSession(
    logger::ILogger& logger,
    EndpointPair endpoint_pair,
    const ACE_SOCK_Stream &stream,
    const ACE_INET_Addr &addr)
    : logger_(logger),
      endpoint_pair_(std::move(endpoint_pair)),
      socket_(stream), 
      server_addr_(addr) {}

AceTcpServerSession::~AceTcpServerSession() {
    close();
}

IOResult AceTcpServerSession::read(Buffer buffer, std::chrono::milliseconds timeout) {

    IOResult result = io_.read(socket_, buffer, timeout);

    if (result.code == IOResultCode::Success && result.count > 0) {
        logger_.log(logger::LogLevel::INFO, "TcpServerSession: %s <- %s received: '%s'",
               endpoint_pair_.local.to_string().c_str(),
               endpoint_pair_.remote.to_string().c_str(),
               std::string(buffer.data, result.count).c_str());
    }

    return result;
}

IOResult AceTcpServerSession::write(ConstBuffer buffer, std::chrono::milliseconds timeout) {
    IOResult result = io_.write(socket_, buffer, timeout);

    if (result.code == IOResultCode::Success && result.count > 0) {
        logger_.log(logger::LogLevel::INFO, "TcpServerSession: %s -> %s sent: '%s'",
               endpoint_pair_.local.to_string().c_str(),
               endpoint_pair_.remote.to_string().c_str(),
               std::string(buffer.data, result.count).c_str());
    }

    return result;
}

void AceTcpServerSession::close() {
    socket_.close();
}

Endpoint AceTcpServerSession::get_peer() const {
    return to_endpoint(server_addr_);
}