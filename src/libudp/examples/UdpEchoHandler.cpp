#include "UdpEchoHandler.h"

#include <common/Constants.h>
#include <common/OwnedBuffer.h>
#include <libacetools/IOResult.h>

#include "common/task/CompletedTask.h"

#include <string>

UdpEchoHandler::UdpEchoHandler(logger::ILogger& logger)
    : logger_(logger) {}

std::unique_ptr<ITask> UdpEchoHandler::handle_client(udp::IUdpSession& client_session) {
    OwnedBuffer buffer(common::STANDARD_BUFFER_SIZE);
    Endpoint sender;

    IOResult result = client_session.read_from(buffer.view(), sender, common::STANDARD_TIMEOUT);
    
    if (result.code == IOResultCode::Error) {
        logger_.log(logger::LogLevel::ERROR, "UdpEchoHandler: failed to read: %s", result.error_message.c_str());
        return std::make_unique<CompletedTask>();
    }
    
    if (result.code == IOResultCode::ConnectionClosed) {
        logger_.log(logger::LogLevel::INFO, "UdpEchoHandler: read ConnectionClosed");
        return std::make_unique<CompletedTask>();
    }
    
    if (result.code == IOResultCode::Timeout) {
        return std::make_unique<CompletedTask>();
    }

    // Prepend 'echo ' to the received message
    std::string received_msg(buffer.view().data, result.count);
    std::string echo_msg = "echo " + received_msg;
    ConstBuffer response_buffer(echo_msg.data(), echo_msg.size());
    
    IOResult write_result = client_session.write_to(response_buffer, sender, common::STANDARD_TIMEOUT);

            logger_.log(logger::LogLevel::INFO, "UdpEchoHandler: %s received: '%s', echoing back: '%s' to %s",
        sender.to_string().c_str(),
        received_msg.c_str(),
        echo_msg.c_str(),
        sender.to_string().c_str());
        
    if (write_result.code == IOResultCode::Error) {
        logger_.log(logger::LogLevel::ERROR, "UdpEchoHandler: failed to write: %s", write_result.error_message.c_str());
        return std::make_unique<CompletedTask>();
    }
    
    if (write_result.code == IOResultCode::ConnectionClosed) {
        logger_.log(logger::LogLevel::INFO, "UdpEchoHandler: write ConnectionClosed");
        return std::make_unique<CompletedTask>();
    }
    
    if (write_result.code == IOResultCode::Timeout) {
        return std::make_unique<CompletedTask>();
    }
    
    return std::make_unique<CompletedTask>();
} 