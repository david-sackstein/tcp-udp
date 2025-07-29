#include "AceUdpServer.h"
#include "AceUdpClientSession.h"

#include <libacetools/ISocketIO.h>
#include <libacetools/Exports.h>

#include <ace/INET_Addr.h>
#include <ace/Log_Msg.h>

#include <iostream>

AceUdpServer::AceUdpServer(
    logger::ILogger &logger,
    const Endpoint &local_endpoint,
    udp::IUdpClientHandler &handler,
    std::shared_ptr<ACE_Reactor> external_reactor)
    : logger_(logger),
      reactor_(external_reactor ? std::move(external_reactor) : std::make_shared<ACE_Reactor>()),
      handler_(handler),
      signal_registration_(register_for_sigint(reactor_.get())),
      local_endpoint_(local_endpoint) {
    const ACE_INET_Addr local_addr = to_ace_addr(local_endpoint_);
    if (socket_.open(local_addr) == -1) {
        throw std::runtime_error("Failed to open UDP socket at " + local_endpoint.to_string());
    }
    local_endpoint_ = get_bound_endpoint(socket_);

    session_ = std::make_unique<AceUdpClientSession>(logger_, local_endpoint_, socket_);

    logger_.log(logger::LogLevel::INFO, "UdpServer: %s successfully bound", local_endpoint_.to_string().c_str());
}

AceUdpServer::~AceUdpServer() {
    stop();
    socket_.close();
    logger_.log(logger::LogLevel::INFO, "UdpServer: %s socket closed", local_endpoint_.to_string().c_str());
}

const Endpoint &AceUdpServer::get_local_endpoint() const {
    return local_endpoint_;
}

void AceUdpServer::start() {
    reactor_->register_handler(this, READ_MASK);
    reactor_->run_reactor_event_loop();
}

void AceUdpServer::stop() {
    if (stopped_) {
        return; // Already stopped
    }
    reactor_->end_reactor_event_loop();
    reactor_->remove_handler(this, READ_MASK);
    stop_all_tasks();
    stopped_ = true;
}

int AceUdpServer::handle_input(ACE_HANDLE) {
    logger_.log(logger::LogLevel::INFO, "UdpServer: received message from %s", local_endpoint_.to_string().c_str());

    auto task = handler_.handle_client(*session_);
    if (task) {
        tasks_.emplace_back(std::move(task));
    }

    return 0;
}

ACE_HANDLE AceUdpServer::get_handle() const {
    return socket_.get_handle();
}

void AceUdpServer::stop_all_tasks() {
    // is_cancelled_ was passed to each task. Tasks are expected to return a short time after
    is_cancelled_ = true;

    for (auto &task: tasks_) {
        task->stop();
    }

    tasks_.clear();
}
