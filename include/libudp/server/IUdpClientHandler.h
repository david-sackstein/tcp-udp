#pragma once

#include <common/task/ITask.h>
#include <libudp/server/IUdpSession.h>

#include <memory>

namespace udp {

class IUdpClientHandler {
public:
    virtual ~IUdpClientHandler() = default;

    virtual std::unique_ptr<ITask> handle_client(IUdpSession& client_session) = 0;
};

} // namespace udp
