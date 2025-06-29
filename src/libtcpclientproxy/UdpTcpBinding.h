#pragma once

#include "libtcp/client/ITcpClient.h"
#include "libtcp/ITcpSession.h"

struct UdpTcpBinding {
    // The client is stored too because it must outlive the tcp_session
    std::unique_ptr<tcp::ITcpClient> client;
    std::shared_ptr<tcp::ITcpSession> tcp_session;
}; 