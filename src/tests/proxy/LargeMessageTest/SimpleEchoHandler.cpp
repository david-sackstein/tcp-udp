#include "SimpleEchoHandler.h"

#include <common/OwnedBuffer.h>
#include <common/task/RunningTask.h>
#include <libacetools/IOResultCode.h>
#include <libtcp/ITcpSession.h>

#include <atomic>
#include <chrono>
#include <memory>

SimpleEchoHandler::SimpleEchoHandler(logger::ILogger& logger) : logger_(logger) {}

std::unique_ptr<ITask> SimpleEchoHandler::handle_client(std::unique_ptr<tcp::ITcpSession> client_session) {
    std::shared_ptr shared_session = std::move(client_session);
    // Capture shared_ptr to this to ensure the handler stays alive
    std::shared_ptr<SimpleEchoHandler> shared_this = std::make_shared<SimpleEchoHandler>(*this);

    return std::make_unique<RunningTask>([shared_session, shared_this](std::atomic<bool>& cancelled) {
        OwnedBuffer buffer_in(BUFFER_SIZE);

        while (!cancelled) {
            auto read_result = shared_session->read(buffer_in.view(), TIMEOUT_MS);

            if (read_result.code == IOResultCode::Error) {
                shared_this->logger_.log(logger::LogLevel::ERROR, "SimpleEchoHandler: failed to read: %s",
                    read_result.error_message.c_str());
                break;
            }

            if (read_result.code == IOResultCode::ConnectionClosed) {
                shared_this->logger_.log(logger::LogLevel::INFO, "SimpleEchoHandler: read ConnectionClosed");
                break;
            }

            if (read_result.code == IOResultCode::Timeout) {
                continue; // Retry read
            }

            // Echo back the exact message without any prefix
            ConstBuffer buffer_out(buffer_in.view().data, read_result.count);

            auto write_result = shared_session->write(buffer_out, TIMEOUT_MS);

            if (write_result.code == IOResultCode::Error) {
                shared_this->logger_.log(logger::LogLevel::ERROR, "SimpleEchoHandler: failed to write: %s",
                    write_result.error_message.c_str());
                break;
            }

            if (write_result.code == IOResultCode::ConnectionClosed) {
                shared_this->logger_.log(logger::LogLevel::INFO, "SimpleEchoHandler: write ConnectionClosed");
                break;
            }

            if (write_result.code == IOResultCode::Timeout) {
                break;
            }
        }
    });
}