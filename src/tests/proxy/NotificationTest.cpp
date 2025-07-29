#include "NotificationTest.h"

#include <libtcp/Exports.h>
#include <libtcpclientproxy/Exports.h>
#include <libtcpserverproxy/Exports.h>
#include <libudp/Exports.h>
#include <liblogger/Exports.h>

#include <common/OwnedBuffer.h>
#include <common/task/RunningTask.h>
#include <libtcp/client/ITcpClient.h>

#include <chrono>
#include <thread>
#include <unordered_map>
#include <vector>
#include <mutex>
#include <string>
#include <sstream>

// Custom handler for multi-client notification test
class NotificationHandler : public tcp::ITcpClientHandler {
public:
    explicit NotificationHandler(logger::ILogger& logger) : logger_(logger) {}

    std::unique_ptr<ITask> handle_client(std::unique_ptr<tcp::ITcpSession> client_session) override {
        std::shared_ptr shared_session = std::move(client_session);
        
        logger_.log("NotificationHandler: New client connected");
        
        // Add this session to our tracked sessions
        {
            std::lock_guard<std::mutex> lock(sessions_mutex_);
            active_sessions_.push_back(shared_session);
            logger_.log("NotificationHandler: Total active sessions: %zu", active_sessions_.size());
        }

        return std::make_unique<RunningTask>([shared_session, this](std::atomic<bool>& cancelled) {
            OwnedBuffer buffer_in(1024);

            while (!cancelled) {
                auto read_result = shared_session->read(buffer_in.view(), std::chrono::milliseconds(100));
                
                if (read_result.code == IOResultCode::Error || 
                    read_result.code == IOResultCode::ConnectionClosed) {
                    logger_.log("NotificationHandler: Client disconnected");
                    break;
                }

                if (read_result.code == IOResultCode::Timeout) {
                    continue;
                }

                std::string received_message(buffer_in.view().data, read_result.count);
                logger_.log("NotificationHandler: Received message: '%s'", received_message.c_str());
                
                // Send echo response to the sender
                std::string echo_response = "echo " + received_message;
                ConstBuffer echo_buffer(echo_response.data(), echo_response.size());
                auto echo_result = shared_session->write(echo_buffer, std::chrono::milliseconds(100));
                logger_.log("NotificationHandler: Sent echo response to sender");
                
                // Send notification to all OTHER clients
                std::string notification = "notify " + received_message;
                ConstBuffer notify_buffer(notification.data(), notification.size());
                
                std::lock_guard<std::mutex> lock(sessions_mutex_);
                logger_.log("NotificationHandler: Sending notifications to %zu other clients", active_sessions_.size() - 1);
                for (auto& session : active_sessions_) {
                    if (session.get() != shared_session.get()) {
                        auto notify_result = session->write(notify_buffer, std::chrono::milliseconds(100));
                        logger_.log("NotificationHandler: Sent notification to other client");
                    }
                }
            }
            
            // Remove this session from active sessions when done
            {
                std::lock_guard<std::mutex> lock(sessions_mutex_);
                auto it = std::find(active_sessions_.begin(), active_sessions_.end(), shared_session);
                if (it != active_sessions_.end()) {
                    active_sessions_.erase(it);
                    logger_.log("NotificationHandler: Removed session, remaining: %zu", active_sessions_.size());
                }
            }
        });
    }

private:
    logger::ILogger& logger_;
    std::vector<std::shared_ptr<tcp::ITcpSession>> active_sessions_;
    std::mutex sessions_mutex_;
};

void MultiClientNotificationTest::SetUp() {
    logger_ = logger::create_console_logger();
}

void MultiClientNotificationTest::TearDown() {
    if (notification_server_) {
        notification_server_->stop();
    }
    if (clientProxy_) {
        clientProxy_->stop();
    }
    if (serverProxy_) {
        serverProxy_->stop();
    }
    
    // Give time for cleanup
    std::this_thread::sleep_for(std::chrono::milliseconds(100));
}

void MultiClientNotificationTest::runNotificationTest(bool useProxies) {
    logger_->log("Starting notification test (useProxies=%s)", useProxies ? "true" : "false");
    
    // Create notification handler and server
    auto notification_handler = std::make_unique<NotificationHandler>(*logger_);
    notification_server_ = tcp::start_tcp_server(
        *logger_,
        Endpoint::loop_back(TCP_SERVER_PORT),
        *notification_handler);
    
    uint16_t client_target_port;
    
    if (useProxies) {
        // Start proxies for the proxy chain test
        serverProxy_ = server_proxy::start_tcp_server_proxy(
            *logger_,
            Endpoint::loop_back(TCP_SERVER_PROXY_PORT),
            Endpoint::loop_back(TCP_CLIENT_PROXY_PORT));
            
        clientProxy_ = client_proxy::start_tcp_client_proxy(
            *logger_,
            Endpoint::loop_back(TCP_CLIENT_PROXY_PORT),
            Endpoint::loop_back(TCP_SERVER_PORT));
        
        // Clients connect to the server proxy
        client_target_port = TCP_SERVER_PROXY_PORT;
        logger_->log("Using proxy chain: clients -> %u -> %u -> %u", 
                    TCP_SERVER_PROXY_PORT, TCP_CLIENT_PROXY_PORT, TCP_SERVER_PORT);
    } else {
        // Direct connection to the notification server
        client_target_port = TCP_SERVER_PORT;
        logger_->log("Using direct connection: clients -> %u", TCP_SERVER_PORT);
    }
    
    // Give time for servers to start
    std::this_thread::sleep_for(std::chrono::milliseconds(500));
    
    // Create 3 TCP clients
    auto client1 = tcp::create_tcp_client(*logger_);
    auto client2 = tcp::create_tcp_client(*logger_);
    auto client3 = tcp::create_tcp_client(*logger_);
    
    // Connect all clients to the target (either proxy or server directly)
    auto session1 = client1->connect(Endpoint::loop_back(TCP_CLIENT1_PORT), Endpoint::loop_back(client_target_port));
    auto session2 = client2->connect(Endpoint::loop_back(TCP_CLIENT2_PORT), Endpoint::loop_back(client_target_port));
    auto session3 = client3->connect(Endpoint::loop_back(TCP_CLIENT3_PORT), Endpoint::loop_back(client_target_port));
    
    ASSERT_TRUE(session1);
    ASSERT_TRUE(session2);
    ASSERT_TRUE(session3);
    
    // Each client sends a message with their index (immediately after connecting for proxy compatibility)
    std::string message1 = "message from client 1";
    std::string message2 = "message from client 2";
    std::string message3 = "message from client 3";
    
    session1->write(ConstBuffer(message1.data(), message1.size()), std::chrono::milliseconds(1000));
    std::this_thread::sleep_for(std::chrono::milliseconds(200));
    session2->write(ConstBuffer(message2.data(), message2.size()), std::chrono::milliseconds(1000));
    std::this_thread::sleep_for(std::chrono::milliseconds(200));
    session3->write(ConstBuffer(message3.data(), message3.size()), std::chrono::milliseconds(1000));
    
    // Give time for messages to propagate
    std::this_thread::sleep_for(std::chrono::milliseconds(1000));
    
    // Each client should receive:
    // - 1 echo response for their own message
    // - 2 notification messages for the other clients' messages
    // Total: 3 messages per client
    
    OwnedBuffer buffer(1024);
    std::vector<std::string> client1_messages, client2_messages, client3_messages;
    
    // Read messages from each client (with timeout to allow async notifications)
    for (int attempt = 0; attempt < 30; ++attempt) {
        // Try reading from each client
        auto result1 = session1->read(buffer.view(), std::chrono::milliseconds(100));
        if (result1.code == IOResultCode::Success && result1.count > 0) {
            client1_messages.emplace_back(buffer.view().data, result1.count);
        }
        
        auto result2 = session2->read(buffer.view(), std::chrono::milliseconds(100));
        if (result2.code == IOResultCode::Success && result2.count > 0) {
            client2_messages.emplace_back(buffer.view().data, result2.count);
        }
        
        auto result3 = session3->read(buffer.view(), std::chrono::milliseconds(100));
        if (result3.code == IOResultCode::Success && result3.count > 0) {
            client3_messages.emplace_back(buffer.view().data, result3.count);
        }
        
        // Check if all clients received at least one message (TCP may concatenate)
        if (client1_messages.size() >= 1 && client2_messages.size() >= 1 && client3_messages.size() >= 1) {
            break;
        }
        
        std::this_thread::sleep_for(std::chrono::milliseconds(100));
    }
    
    // Log what we received for debugging
    logger_->log("Client 1 received %zu messages", client1_messages.size());
    for (const auto& msg : client1_messages) {
        logger_->log("  Client 1: '%s'", msg.c_str());
    }
    
    logger_->log("Client 2 received %zu messages", client2_messages.size());
    for (const auto& msg : client2_messages) {
        logger_->log("  Client 2: '%s'", msg.c_str());
    }
    
    logger_->log("Client 3 received %zu messages", client3_messages.size());
    for (const auto& msg : client3_messages) {
        logger_->log("  Client 3: '%s'", msg.c_str());
    }
    
    // Verify each client received at least one message
    ASSERT_GE(client1_messages.size(), 1);
    ASSERT_GE(client2_messages.size(), 1);  
    ASSERT_GE(client3_messages.size(), 1);
    
    // Combine all messages from each client (in case TCP split them)
    std::string client1_combined;
    for (const auto& msg : client1_messages) {
        client1_combined += msg;
    }
    
    std::string client2_combined;
    for (const auto& msg : client2_messages) {
        client2_combined += msg;
    }
    
    std::string client3_combined;
    for (const auto& msg : client3_messages) {
        client3_combined += msg;
    }
    
    // Verify client 1 received: echo for its own message + notifications for others
    EXPECT_TRUE(client1_combined.find("echo " + message1) != std::string::npos);
    EXPECT_TRUE(client1_combined.find("notify " + message2) != std::string::npos);
    EXPECT_TRUE(client1_combined.find("notify " + message3) != std::string::npos);
    
    // Verify client 2 received: echo for its own message + notifications for others
    EXPECT_TRUE(client2_combined.find("echo " + message2) != std::string::npos);
    EXPECT_TRUE(client2_combined.find("notify " + message1) != std::string::npos);
    EXPECT_TRUE(client2_combined.find("notify " + message3) != std::string::npos);
    
    // Verify client 3 received: echo for its own message + notifications for others
    EXPECT_TRUE(client3_combined.find("echo " + message3) != std::string::npos);
    EXPECT_TRUE(client3_combined.find("notify " + message1) != std::string::npos);
    EXPECT_TRUE(client3_combined.find("notify " + message2) != std::string::npos);
    
    logger_->log("Notification test completed successfully (useProxies=%s)", useProxies ? "true" : "false");
}

// Test with direct connection (no proxies)
TEST_F(MultiClientNotificationTest, DirectConnection) {
    runNotificationTest(false);
}

// Test with proxy chain  
TEST_F(MultiClientNotificationTest, ProxyChainWithNotifications) {
    runNotificationTest(true);
} 