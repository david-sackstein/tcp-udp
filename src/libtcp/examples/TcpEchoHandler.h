#pragma once

#include <libacetools/IOResult.h>
#include <liblogger/ILogger.h>
#include <libtcp/server/ITcpClientHandler.h>

class TcpEchoHandler final : public tcp::ITcpClientHandler {
public:
    explicit TcpEchoHandler(logger::ILogger& logger);
    std::unique_ptr<ITask> handle_client(std::unique_ptr<tcp::ITcpSession> session) override;

private:
    std::string processEchoMessage(const std::string& received_message) const;
    bool handleReadResult(const IOResult& read_result) const;
    bool handleWriteResult(const IOResult& write_result) const;

    logger::ILogger& logger_;
};
