#pragma once

#include <common/task/ITask.h>
#include <libtcp/ITcpSession.h>

#include <memory>

namespace tcp {
class ITcpClientHandler {
public:
    virtual ~ITcpClientHandler() = default;

    virtual std::unique_ptr<ITask> handle_client(std::unique_ptr<ITcpSession> client_session) = 0;
};
} // namespace tcp
