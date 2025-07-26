#include "TcpEchoHandler.h"

#include <common/OwnedBuffer.h>
#include <common/task/RunningTask.h>

#include <memory>
#include <string>

TcpEchoHandler::TcpEchoHandler(logger::ILogger& logger)
    : logger_(logger) {}

std::unique_ptr<ITask> TcpEchoHandler::handle_client(std::unique_ptr<tcp::ITcpSession> client_session) {
    std::shared_ptr shared_session = std::move(client_session);

    return std::make_unique<RunningTask>([shared_session, this](std::atomic<bool>& cancelled) {
        OwnedBuffer buffer_in(1024);

        while (!cancelled) {
            auto read_result = shared_session->read(buffer_in.view(), std::chrono::milliseconds(100));
            
            if (read_result.code == IOResultCode::Error) {
                logger_.log("TcpEchoHandler: failed to read: %s", read_result.error_message.c_str());
                break;
            }

            if (read_result.code == IOResultCode::ConnectionClosed) {
                logger_.log("TcpEchoHandler: read ConnectionClosed");
                break;
            }

            if (read_result.code == IOResultCode::Timeout) {
                continue; // Retry read
            }

            // Create response with "echo " prepended to the received message
            std::string received_message(buffer_in.view().data, read_result.count);
            std::string echo_response = "echo " + received_message;
            
            // Create buffer for the echo response
            ConstBuffer buffer_out(echo_response.data(), echo_response.size());

            auto write_result = shared_session->write(buffer_out, std::chrono::milliseconds(100));
            
            if (write_result.code == IOResultCode::Error) {
                logger_.log("TcpEchoHandler: failed to write: %s", write_result.error_message.c_str());
                break;
            }

            if (write_result.code == IOResultCode::ConnectionClosed) {
                logger_.log("TcpEchoHandler: write ConnectionClosed");
                break;
            }

            if (write_result.code == IOResultCode::Timeout) {
                break;
            }
        }
    });
}
