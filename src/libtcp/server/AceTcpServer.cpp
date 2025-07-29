#include "AceTcpServer.h"

#include <libacetools/Converters.h>

#include <ace/Log_Msg.h>
#include <libacetools/Exports.h>

AceTcpServer::AceTcpServer(logger::ILogger& logger, const Endpoint& local_endpoint, tcp::ITcpClientHandler &handler)
    : logger_(logger),
      handler_(handler),
      external_reactor_(nullptr),
      acceptor_(logger, get_reactor(), handler_),
      signal_registration_(register_for_sigint(&get_reactor()))
{
    if (acceptor_.open(local_endpoint) != 0) {
        ACE_ERROR((LM_ERROR, ACE_TEXT("Failed to open acceptor on %s, errno: %d (%s)\n"), 
                   local_endpoint.to_string().c_str(), 
                   ACE_OS::last_error(),
                   ACE_OS::strerror(ACE_OS::last_error())));
        throw std::runtime_error("Failed to open acceptor on " + local_endpoint.to_string());
    }
    local_endpoint_ = acceptor_.get_local_endpoint();

    logger_.log("TcpServer: %s successfully bound", local_endpoint_.to_string().c_str());
}

AceTcpServer::AceTcpServer(logger::ILogger& logger, const Endpoint& local_endpoint, tcp::ITcpClientHandler &handler, std::shared_ptr<ACE_Reactor> external_reactor)
    : logger_(logger),
      handler_(handler),
      external_reactor_(std::move(external_reactor)),
      acceptor_(logger, get_reactor(), handler_),
      signal_registration_(register_for_sigint(&get_reactor()))
{
    if (acceptor_.open(local_endpoint) != 0) {
        ACE_ERROR((LM_ERROR, ACE_TEXT("Failed to open acceptor on %s, errno: %d (%s)\n"), 
                   local_endpoint.to_string().c_str(), 
                   ACE_OS::last_error(),
                   ACE_OS::strerror(ACE_OS::last_error())));
        throw std::runtime_error("Failed to open acceptor on " + local_endpoint.to_string());
    }
    local_endpoint_ = acceptor_.get_local_endpoint();

    logger_.log("TcpServer: %s successfully bound with external reactor", local_endpoint_.to_string().c_str());
}

ACE_Reactor& AceTcpServer::get_reactor() {
    return external_reactor_ ? *external_reactor_ : reactor_;
}

void AceTcpServer::start() {
    // blocks thread
    int status = get_reactor().run_reactor_event_loop();
    if (status != 0) {
        ACE_ERROR((LM_ERROR, ACE_TEXT("run_reactor_event_loop returned: %d, errno: %d\n"),
                   status, ACE_OS::last_error()));
    }
}

void AceTcpServer::stop() {
    acceptor_.close();
    get_reactor().end_reactor_event_loop();
    logger_.log("TcpServer: %s stopped", local_endpoint_.to_string().c_str());
}
