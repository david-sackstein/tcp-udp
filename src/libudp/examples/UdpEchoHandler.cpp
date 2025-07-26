#include "UdpEchoHandler.h"

#include <libacetools/IOResult.h>
#include <common/OwnedBuffer.h>

#include "common/task/CompletedTask.h"

#include <string>

std::unique_ptr<ITask> UdpEchoHandler::handle_client(udp::IUdpSession& client_session) {
    OwnedBuffer buffer(1024);
    Endpoint sender;

    IOResult result = client_session.read_from(buffer.view(), sender, std::chrono::milliseconds(1000));
    
    if (result.code == IOResultCode::Error) {
        printf("UdpEchoHandler: failed to read: %s\n", result.error_message.c_str());
        return std::make_unique<CompletedTask>();
    }
    
    if (result.code == IOResultCode::ConnectionClosed || result.code == IOResultCode::Timeout) {
        return std::make_unique<CompletedTask>();
    }

    // Prepend 'echo ' to the received message
    std::string received_msg(buffer.view().data, result.count);
    std::string echo_msg = "echo " + received_msg;
    ConstBuffer response_buffer(echo_msg.data(), echo_msg.size());
    
    IOResult write_result = client_session.write_to(response_buffer, sender, std::chrono::milliseconds(1000));

    printf("UdpEchoHandler: %s received: '%s', echoing back: '%s' to %s\n",
        sender.to_string().c_str(),
        received_msg.c_str(),
        echo_msg.c_str(),
        sender.to_string().c_str());
        
    if (write_result.code == IOResultCode::Error) {
        printf("UdpEchoHandler: failed to write: %s\n", write_result.error_message.c_str());
        return std::make_unique<CompletedTask>();
    }
    
    if (write_result.code == IOResultCode::ConnectionClosed || write_result.code == IOResultCode::Timeout) {
        return std::make_unique<CompletedTask>(); // Graceful exit
    }
    
    return std::make_unique<CompletedTask>();
} 