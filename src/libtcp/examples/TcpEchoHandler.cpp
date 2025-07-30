#include "TcpEchoHandler.h"

#include <common/Constants.h>
#include <common/OwnedBuffer.h>
#include <common/task/RunningTask.h>

#include <memory>

#include <string>

TcpEchoHandler::TcpEchoHandler(logger::ILogger& logger) : logger_(logger) {}

std::unique_ptr<ITask> TcpEchoHandler::handle_client(std::unique_ptr<tcp::ITcpSession> client_session) {
    std::shared_ptr shared_session = std::move(client_session);

    return std::make_unique<RunningTask>([shared_session, this](std::atomic<bool>& cancelled) {
        OwnedBuffer buffer_in(common::STANDARD_BUFFER_SIZE);

        while (!cancelled) {
            auto read_result = shared_session->read(buffer_in.view(), common::SHORT_TIMEOUT);

            if (read_result.code == IOResultCode::Timeout) {
                continue; // Retry read
            }

            std::string received_message(buffer_in.view().data, read_result.count);
            if (!handleReadResult(read_result)) {
                break;
            }

            std::string echo_response = processEchoMessage(received_message);
            ConstBuffer buffer_out(echo_response.data(), echo_response.size());

            auto write_result = shared_session->write(buffer_out, common::SHORT_TIMEOUT);

            if (!handleWriteResult(write_result)) {
                break;
            }
        }
    });
}

std::string TcpEchoHandler::processEchoMessage(const std::string& received_message) const {
    return "echo [" + received_message + "]";
}

bool TcpEchoHandler::handleReadResult(const IOResult& read_result) const {
    if (read_result.code == IOResultCode::Error) {
        logger_.log(logger::LogLevel::ERROR, "TcpEchoHandler: failed to read: %s", read_result.error_message.c_str());
        return false;
    }

    if (read_result.code == IOResultCode::ConnectionClosed) {
        logger_.log(logger::LogLevel::INFO, "TcpEchoHandler: read ConnectionClosed");
        return false;
    }

    return true;
}

bool TcpEchoHandler::handleWriteResult(const IOResult& write_result) const {
    if (write_result.code == IOResultCode::Error) {
        logger_.log(logger::LogLevel::ERROR, "TcpEchoHandler: failed to write: %s", write_result.error_message.c_str());
        return false;
    }

    if (write_result.code == IOResultCode::ConnectionClosed) {
        logger_.log(logger::LogLevel::INFO, "TcpEchoHandler: write ConnectionClosed");
        return false;
    }

    if (write_result.code == IOResultCode::Timeout) {
        return false;
    }

    return true;
}
