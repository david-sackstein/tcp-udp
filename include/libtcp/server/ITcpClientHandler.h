#pragma once

#include <libtcp/ITcpSession.h>
#include <common/task/ITask.h>

#include <memory>

namespace tcp {
    class ITcpClientHandler {
    public:
        virtual ~ITcpClientHandler() = default;

        virtual std::unique_ptr<ITask> handle_client(std::unique_ptr<tcp::ITcpSession> client_session) = 0;
    };
}
