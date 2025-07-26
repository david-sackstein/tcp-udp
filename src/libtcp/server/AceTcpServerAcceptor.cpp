#include "AceTcpServerAcceptor.h"
#include "AceTcpClientSession.h"

#include <libacetools/Exports.h>

#include <ace/Log_Msg.h>
#include <ace/INET_Addr.h>
#include <ace/Reactor.h>

#include <iostream>

AceTcpServerAcceptor::AceTcpServerAcceptor(logger::ILogger &logger, ACE_Reactor &reactor, tcp::ITcpClientHandler &handler)
    : reactor_(reactor),
      client_handler_(handler),
      logger_(logger) {
}

int AceTcpServerAcceptor::open(const Endpoint &local_endpoint) {
    ACE_INET_Addr listen_addr = to_ace_addr(local_endpoint);

    if (acceptor_.open(listen_addr, 1) == -1) {
        ACE_ERROR_RETURN(
            (LM_ERROR, ACE_TEXT("Failed to open acceptor on %s, errno: %d (%s)\n"),
                local_endpoint.to_string().c_str(),
                ACE_OS::last_error(),
                ACE_OS::strerror(ACE_OS::last_error())), -1);
    }

    // Update to the actual bound address (handles ephemeral port)
    local_endpoint_ = get_bound_endpoint(acceptor_);

    if (reactor_.register_handler(this, ACE_Event_Handler::READ_MASK) == -1) {
        ACE_ERROR((LM_ERROR, ACE_TEXT("Failed to register handler with reactor, errno: %d (%s)\n"),
            ACE_OS::last_error(),
            ACE_OS::strerror(ACE_OS::last_error())));
        acceptor_.close();
        return -1;
    }

    return 0;
}

int AceTcpServerAcceptor::handle_input(ACE_HANDLE) {
    ACE_SOCK_Stream client_socket{};
    ACE_INET_Addr client_addr{};

    if (acceptor_.accept(client_socket, &client_addr) == -1) {
        ACE_ERROR((LM_ERROR, ACE_TEXT("Failed to accept new client\n")));
        return -1;
    }

    // Set linger timeout to prevent socket lingering
    get_socket_io().set_linger_timeout(client_socket);

    auto client_session = std::make_unique<AceTcpClientSession>(
        logger_, local_endpoint_, client_socket, client_addr);

    std::unique_ptr<ITask> connection_task = client_handler_.handle_client(std::move(client_session));

    if (connection_task) {
        tasks_.emplace_back(std::move(connection_task));
    }
    return 0;
}

int AceTcpServerAcceptor::close() {
    if (reactor_.remove_handler(this, ACE_Event_Handler::READ_MASK) == -1) {
        ACE_ERROR((LM_ERROR, ACE_TEXT("Failed to remove handler from reactor\n")));
    }

    if (acceptor_.close() == -1) {
        ACE_ERROR((LM_ERROR, ACE_TEXT("Failed to close acceptor\n")));
    }

    stop_all_tasks();
    return 0;
}

void AceTcpServerAcceptor::stop_all_tasks() {
    // is_cancelled_ was passed to each task. Tasks are expected to return a short time after
    is_cancelled_ = true;

    for (auto &task: tasks_) {
        task->stop();
    }

    tasks_.clear();
}
