#include "AceTcpServer.h"

#include <libacetools/Converters.h>

#include <ace/Log_Msg.h>
#include <libacetools/Exports.h>

AceTcpServer::AceTcpServer(const Endpoint& local_endpoint, tcp::ITcpClientHandler &handler)
    : handler_(handler),
      acceptor_(reactor_, handler_),
      signal_registration_(register_for_sigint(&reactor_))
{
    if (acceptor_.open(local_endpoint) != 0) {
        ACE_ERROR((LM_ERROR, ACE_TEXT("Failed to open acceptor on %s, errno: %d (%s)\n"), 
                   local_endpoint.to_string().c_str(), 
                   ACE_OS::last_error(),
                   ACE_OS::strerror(ACE_OS::last_error())));
        throw std::runtime_error("Failed to open acceptor on " + local_endpoint.to_string());
    }
    local_endpoint_ = acceptor_.get_local_endpoint();

    printf("TcpServer: %s successfully bound\n", local_endpoint_.to_string().c_str());
}

void AceTcpServer::start() {
    // blocks thread
    int status = reactor_.run_reactor_event_loop();
    if (status != 0) {
        ACE_ERROR((LM_ERROR, ACE_TEXT("run_reactor_event_loop returned: %d, errno: %d\n"),
                   status, ACE_OS::last_error()));
    }
}

void AceTcpServer::stop() {
    acceptor_.close();
    reactor_.end_reactor_event_loop();
    printf("TcpServer: %s stopped\n", local_endpoint_.to_string().c_str());
}
