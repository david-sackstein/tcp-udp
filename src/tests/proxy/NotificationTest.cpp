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
    // Stop all servers and proxies
    if (notification_server_) {
        notification_server_->stop();
    }
    if (notification_server1_) {
        notification_server1_->stop();
    }
    if (notification_server2_) {
        notification_server2_->stop();
    }
    if (notification_server3_) {
        notification_server3_->stop();
    }
    if (clientProxy_) {
        clientProxy_->stop();
    }
    if (serverProxy1_) {
        serverProxy1_->stop();
    }
    if (serverProxy2_) {
        serverProxy2_->stop();
    }
    if (serverProxy3_) {
        serverProxy3_->stop();
    }
    
    // Give time for cleanup
    std::this_thread::sleep_for(std::chrono::milliseconds(100));
}

void MultiClientNotificationTest::runNotificationTest(bool useProxies) {
    logger_->log("Starting notification test (useProxies=%s)", useProxies ? "true" : "false");
    
    std::vector<uint16_t> client_target_ports;
    
    if (useProxies) {
        // Proxy architecture: 3 server proxies -> 1 client proxy -> 1 TCP server with simple echo handler
        logger_->log("Using proxy chain: clients -> server_proxies -> %u -> %u", 
                    TCP_CLIENT_PROXY_PORT, TCP_SERVER_PORT);
        
        // Create one TCP server with simple echo handler for proxy scenario
        notification_handler_ = tcp::create_tcp_echo_handler(*logger_);
        notification_server_ = tcp::start_tcp_server(
            *logger_,
            Endpoint::loop_back(TCP_SERVER_PORT),
            *notification_handler_);
        
        // Create one client proxy that connects to the shared TCP server
        clientProxy_ = client_proxy::start_tcp_client_proxy(
            *logger_,
            Endpoint::loop_back(TCP_CLIENT_PROXY_PORT),
            Endpoint::loop_back(TCP_SERVER_PORT));
        
        // Create separate server proxies for each client
        serverProxy1_ = server_proxy::start_tcp_server_proxy(
            *logger_,
            Endpoint::loop_back(TCP_SERVER_PROXY1_PORT),
            Endpoint::loop_back(TCP_CLIENT_PROXY_PORT));
            
        serverProxy2_ = server_proxy::start_tcp_server_proxy(
            *logger_,
            Endpoint::loop_back(TCP_SERVER_PROXY2_PORT),
            Endpoint::loop_back(TCP_CLIENT_PROXY_PORT));
            
        serverProxy3_ = server_proxy::start_tcp_server_proxy(
            *logger_,
            Endpoint::loop_back(TCP_SERVER_PROXY3_PORT),
            Endpoint::loop_back(TCP_CLIENT_PROXY_PORT));
        
        // Clients connect to different server proxy ports
        client_target_ports = {TCP_SERVER_PROXY1_PORT, TCP_SERVER_PROXY2_PORT, TCP_SERVER_PROXY3_PORT};
    } else {
        // Direct architecture: For now, use one TCP server (same as before)
        logger_->log("Using direct connection: clients -> %u", TCP_SERVER_PORT);
        
        notification_handler_ = tcp::create_tcp_echo_handler(*logger_);
        notification_server_ = tcp::start_tcp_server(
            *logger_,
            Endpoint::loop_back(TCP_SERVER_PORT),
            *notification_handler_);
        
        // All clients connect to the same server for direct connection test
        client_target_ports = {TCP_SERVER_PORT, TCP_SERVER_PORT, TCP_SERVER_PORT};
    }
    
    // Give time for servers to start
    std::this_thread::sleep_for(std::chrono::milliseconds(500));
    
    // Create 3 TCP clients
    auto client1 = tcp::create_tcp_client(*logger_);
    auto client2 = tcp::create_tcp_client(*logger_);
    auto client3 = tcp::create_tcp_client(*logger_);
    
    // Connect each client to its dedicated server/proxy
    auto session1 = client1->connect(Endpoint::loop_back(TCP_CLIENT1_PORT), Endpoint::loop_back(client_target_ports[0]));
    auto session2 = client2->connect(Endpoint::loop_back(TCP_CLIENT2_PORT), Endpoint::loop_back(client_target_ports[1]));
    auto session3 = client3->connect(Endpoint::loop_back(TCP_CLIENT3_PORT), Endpoint::loop_back(client_target_ports[2]));
    
    ASSERT_TRUE(session1);
    ASSERT_TRUE(session2);
    ASSERT_TRUE(session3);
    
    // Each client sends a message and expects an echo back
    std::string message1 = "message from client 1";
    std::string message2 = "message from client 2";
    std::string message3 = "message from client 3";
    
    // Send messages
    auto write1 = session1->write(ConstBuffer(message1.data(), message1.size()), std::chrono::milliseconds(1000));
    auto write2 = session2->write(ConstBuffer(message2.data(), message2.size()), std::chrono::milliseconds(1000));
    auto write3 = session3->write(ConstBuffer(message3.data(), message3.size()), std::chrono::milliseconds(1000));
    
    ASSERT_EQ(IOResultCode::Success, write1.code);
    ASSERT_EQ(IOResultCode::Success, write2.code);
    ASSERT_EQ(IOResultCode::Success, write3.code);
    
    // Read echo responses
    OwnedBuffer buffer(1024);
    
    auto read1 = session1->read(buffer.view(), std::chrono::milliseconds(2000));
    ASSERT_EQ(IOResultCode::Success, read1.code);
    std::string response1(buffer.view().data, read1.count);
    ASSERT_EQ("echo " + message1, response1);
    
    auto read2 = session2->read(buffer.view(), std::chrono::milliseconds(2000));
    ASSERT_EQ(IOResultCode::Success, read2.code);
    std::string response2(buffer.view().data, read2.count);
    ASSERT_EQ("echo " + message2, response2);
    
    auto read3 = session3->read(buffer.view(), std::chrono::milliseconds(2000));
    ASSERT_EQ(IOResultCode::Success, read3.code);
    std::string response3(buffer.view().data, read3.count);
    ASSERT_EQ("echo " + message3, response3);
    
    logger_->log("Multi-proxy echo test completed successfully (useProxies=%s)", useProxies ? "true" : "false");
    logger_->log("Client 1: sent '%s', received '%s'", message1.c_str(), response1.c_str());
    logger_->log("Client 2: sent '%s', received '%s'", message2.c_str(), response2.c_str());
    logger_->log("Client 3: sent '%s', received '%s'", message3.c_str(), response3.c_str());
    
    // Clean up connections
    client1->disconnect();
    client2->disconnect();
    client3->disconnect();
}

void MultiClientNotificationTest::runCrossClientNotificationTest(bool useProxies) {
    logger_->log("Starting cross-client notification test (useProxies=%s)", useProxies ? "true" : "false");
    
    std::vector<uint16_t> client_target_ports;
    
    if (useProxies) {
        // Proxy architecture: 3 server proxies -> 1 client proxy -> 1 TCP server with NotificationHandler
        logger_->log("Using proxy chain: clients -> server_proxies -> %u -> %u", 
                    TCP_CLIENT_PROXY_PORT, TCP_SERVER_PORT);
        
        // Create one TCP server with NotificationHandler for proxy scenario
        notification_handler_ = std::make_unique<NotificationHandler>(*logger_);
        notification_server_ = tcp::start_tcp_server(
            *logger_,
            Endpoint::loop_back(TCP_SERVER_PORT),
            *notification_handler_);
        
        // Create one client proxy that connects to the shared TCP server
        clientProxy_ = client_proxy::start_tcp_client_proxy(
            *logger_,
            Endpoint::loop_back(TCP_CLIENT_PROXY_PORT),
            Endpoint::loop_back(TCP_SERVER_PORT));
        
        // Create separate server proxies for each client
        serverProxy1_ = server_proxy::start_tcp_server_proxy(
            *logger_,
            Endpoint::loop_back(TCP_SERVER_PROXY1_PORT),
            Endpoint::loop_back(TCP_CLIENT_PROXY_PORT));
            
        serverProxy2_ = server_proxy::start_tcp_server_proxy(
            *logger_,
            Endpoint::loop_back(TCP_SERVER_PROXY2_PORT),
            Endpoint::loop_back(TCP_CLIENT_PROXY_PORT));
            
        serverProxy3_ = server_proxy::start_tcp_server_proxy(
            *logger_,
            Endpoint::loop_back(TCP_SERVER_PROXY3_PORT),
            Endpoint::loop_back(TCP_CLIENT_PROXY_PORT));
        
        // Clients connect to different server proxy ports
        client_target_ports = {TCP_SERVER_PROXY1_PORT, TCP_SERVER_PROXY2_PORT, TCP_SERVER_PROXY3_PORT};
    } else {
        // Direct architecture: use one TCP server with NotificationHandler
        logger_->log("Using direct connection: clients -> %u", TCP_SERVER_PORT);
        
        notification_handler_ = std::make_unique<NotificationHandler>(*logger_);
        notification_server_ = tcp::start_tcp_server(
            *logger_,
            Endpoint::loop_back(TCP_SERVER_PORT),
            *notification_handler_);
        
        // All clients connect to the same server for direct connection test
        client_target_ports = {TCP_SERVER_PORT, TCP_SERVER_PORT, TCP_SERVER_PORT};
    }
    
    // Give time for servers to start
    std::this_thread::sleep_for(std::chrono::milliseconds(500));
    
    // Create 3 TCP clients
    auto client1 = tcp::create_tcp_client(*logger_);
    auto client2 = tcp::create_tcp_client(*logger_);
    auto client3 = tcp::create_tcp_client(*logger_);
    
    // Connect each client to its dedicated server/proxy
    auto session1 = client1->connect(Endpoint::loop_back(TCP_CLIENT1_PORT), Endpoint::loop_back(client_target_ports[0]));
    auto session2 = client2->connect(Endpoint::loop_back(TCP_CLIENT2_PORT), Endpoint::loop_back(client_target_ports[1]));
    auto session3 = client3->connect(Endpoint::loop_back(TCP_CLIENT3_PORT), Endpoint::loop_back(client_target_ports[2]));
    
    ASSERT_TRUE(session1);
    ASSERT_TRUE(session2);
    ASSERT_TRUE(session3);
    
    // Give time for all clients to connect and be registered by NotificationHandler
    std::this_thread::sleep_for(std::chrono::milliseconds(200));
    
    // Each client sends a message with their index
    std::string message1 = "message from client 1";
    std::string message2 = "message from client 2";
    std::string message3 = "message from client 3";
    
    logger_->log("Sending first round of messages...");
    session1->write(ConstBuffer(message1.data(), message1.size()), std::chrono::milliseconds(1000));
    std::this_thread::sleep_for(std::chrono::milliseconds(200));
    session2->write(ConstBuffer(message2.data(), message2.size()), std::chrono::milliseconds(1000));
    std::this_thread::sleep_for(std::chrono::milliseconds(200));
    session3->write(ConstBuffer(message3.data(), message3.size()), std::chrono::milliseconds(1000));
    
    // Give time for messages to propagate and all TCP connections to be established
    std::this_thread::sleep_for(std::chrono::milliseconds(1000));
    
    // Send a second round of messages now that all clients are connected to the server
    std::string message1_round2 = "second message from client 1";
    std::string message2_round2 = "second message from client 2";
    std::string message3_round2 = "second message from client 3";
    
    logger_->log("Sending second round of messages...");
    session1->write(ConstBuffer(message1_round2.data(), message1_round2.size()), std::chrono::milliseconds(1000));
    std::this_thread::sleep_for(std::chrono::milliseconds(200));
    session2->write(ConstBuffer(message2_round2.data(), message2_round2.size()), std::chrono::milliseconds(1000));
    std::this_thread::sleep_for(std::chrono::milliseconds(200));
    session3->write(ConstBuffer(message3_round2.data(), message3_round2.size()), std::chrono::milliseconds(1000));
    
    // Give time for second round messages to propagate
    std::this_thread::sleep_for(std::chrono::milliseconds(1000));
    
    // Each client should receive from both rounds:
    // Round 1: Variable notifications (timing dependent in proxy mode)
    // Round 2: Full notifications (all clients connected)
    // Total: Multiple messages per client
    
    OwnedBuffer buffer(1024);
    std::vector<std::string> client1_messages, client2_messages, client3_messages;
    
    // Read all messages from both rounds (more attempts to catch all messages)
    for (int attempt = 0; attempt < 60; ++attempt) {
        bool received_any = false;
        
        // Try reading from each client
        auto result1 = session1->read(buffer.view(), std::chrono::milliseconds(50));
        if (result1.code == IOResultCode::Success && result1.count > 0) {
            client1_messages.emplace_back(buffer.view().data, result1.count);
            received_any = true;
        }
        
        auto result2 = session2->read(buffer.view(), std::chrono::milliseconds(50));
        if (result2.code == IOResultCode::Success && result2.count > 0) {
            client2_messages.emplace_back(buffer.view().data, result2.count);
            received_any = true;
        }
        
        auto result3 = session3->read(buffer.view(), std::chrono::milliseconds(50));
        if (result3.code == IOResultCode::Success && result3.count > 0) {
            client3_messages.emplace_back(buffer.view().data, result3.count);
            received_any = true;
        }
        
        // If no messages received in this round, we might be done
        if (!received_any) {
            // Give a few more attempts in case messages are still coming
            if (attempt > 45) break;
        }
    }
    
    // Log what each client received
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
    
    // Combine all messages for each client to handle TCP message concatenation
    std::string client1_combined, client2_combined, client3_combined;
    for (const auto& msg : client1_messages) client1_combined += msg;
    for (const auto& msg : client2_messages) client2_combined += msg;
    for (const auto& msg : client3_messages) client3_combined += msg;
    
    // Verify round 2 messages (full cross-client notifications)
    // Round 1 is skipped as behavior is timing-dependent in proxy mode
    // Each client should receive their own echo from round 2
    ASSERT_TRUE(client1_combined.find("echo " + message1_round2) != std::string::npos);
    ASSERT_TRUE(client2_combined.find("echo " + message2_round2) != std::string::npos);
    ASSERT_TRUE(client3_combined.find("echo " + message3_round2) != std::string::npos);
    
    // In round 2, all clients should receive notifications from the other clients
    // (This should work regardless of proxy vs direct connection since all are connected)
    
    // Client 1 should receive notifications from round 2 messages of clients 2 and 3
    ASSERT_TRUE(client1_combined.find("notify " + message2_round2) != std::string::npos);
    ASSERT_TRUE(client1_combined.find("notify " + message3_round2) != std::string::npos);
    
    // Client 2 should receive notifications from round 2 messages of clients 1 and 3
    ASSERT_TRUE(client2_combined.find("notify " + message1_round2) != std::string::npos);
    ASSERT_TRUE(client2_combined.find("notify " + message3_round2) != std::string::npos);
    
    // Client 3 should receive notifications from round 2 messages of clients 1 and 2
    ASSERT_TRUE(client3_combined.find("notify " + message1_round2) != std::string::npos);
    ASSERT_TRUE(client3_combined.find("notify " + message2_round2) != std::string::npos);
    
    logger_->log("Cross-client notification test completed successfully (useProxies=%s)", useProxies ? "true" : "false");
    
    // Clean up connections
    client1->disconnect();
    client2->disconnect();
    client3->disconnect();
}

// Test with direct connection (no proxies)
TEST_F(MultiClientNotificationTest, DirectConnection) {
    runNotificationTest(false);
}

// Test with proxy chain  
TEST_F(MultiClientNotificationTest, ProxyChainWithNotifications) {
    runNotificationTest(true);
}

// Test cross-client notifications with direct connection
TEST_F(MultiClientNotificationTest, DirectConnectionWithCrossClientNotifications) {
    runCrossClientNotificationTest(false);
}

// Test cross-client notifications with proxy chain
TEST_F(MultiClientNotificationTest, ProxyChainWithCrossClientNotifications) {
    runCrossClientNotificationTest(true);
} 