#pragma once

#include <libtcp/server/ITcpClientHandler.h>
#include <common/task/ITask.h>

#include <ace/Event_Handler.h>
#include <ace/SOCK_Acceptor.h>

#include <vector>
#include <atomic>

class AceTcpServerAcceptor final : public ACE_Event_Handler {
public:
    AceTcpServerAcceptor(ACE_Reactor& reactor, tcp::ITcpClientHandler& handler);

    int open(const Endpoint& local_endpoint);
    int close();

    int handle_input(ACE_HANDLE) override;

    [[nodiscard]] ACE_HANDLE get_handle() const override { return acceptor_.get_handle(); }

    [[nodiscard]] const Endpoint& get_local_endpoint() const { return local_endpoint_; }

private:

    void stop_all_tasks();

    ACE_Reactor& reactor_;
    tcp::ITcpClientHandler& client_handler_;

    ACE_SOCK_Acceptor acceptor_;
    Endpoint local_endpoint_{};

    std::vector<std::unique_ptr<ITask>> tasks_;

    // This flag is passed by reference to handlers for cancellation
    [[maybe_unused]] std::atomic<bool> is_cancelled_{false};
};
