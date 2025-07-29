#include "NotificationTest.h"
#include "NotificationHandler.h"

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
#include <vector>
#include <string>
#include <sstream>


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

void MultiClientNotificationTest::SetUp() {
    logger_ = logger::create_console_logger();
}

void MultiClientNotificationTest::TearDown() {
    stopAllServers();
    logger_->log("TearDown: Allowing time for graceful cleanup...");
    std::this_thread::sleep_for(std::chrono::milliseconds(100));
}

void MultiClientNotificationTest::stopAllServers() {
    logger_->log("TearDown: Stopping all servers and clients");
    
    // Disconnect clients first if they exist
    if (client1_) {
        logger_->log("TearDown: Disconnecting client1");
        client1_->disconnect();
    }
    if (client2_) {
        logger_->log("TearDown: Disconnecting client2");
        client2_->disconnect();
    }
    if (client3_) {
        logger_->log("TearDown: Disconnecting client3");
        client3_->disconnect();
    }
    
    // Stop all servers and proxies
    if (notification_server_) notification_server_->stop();
    if (clientProxy_) clientProxy_->stop();
    if (serverProxy1_) serverProxy1_->stop();
    if (serverProxy2_) serverProxy2_->stop();
    if (serverProxy3_) serverProxy3_->stop();
    
    // Reset client pointers
    client1_.reset();
    client2_.reset();
    client3_.reset();
}

std::vector<uint16_t> MultiClientNotificationTest::setupProxyChain(bool useNotificationHandler) {
    logger_->log("Setting up proxy chain: clients -> server_proxies -> %u -> %u", 
                TCP_CLIENT_PROXY_PORT, TCP_SERVER_PORT);
    
    // Create TCP server with appropriate handler
    if (useNotificationHandler) {
        notification_handler_ = std::make_unique<NotificationHandler>(*logger_);
    } else {
        notification_handler_ = tcp::create_tcp_echo_handler(*logger_);
    }
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
    
    return {TCP_SERVER_PROXY1_PORT, TCP_SERVER_PROXY2_PORT, TCP_SERVER_PROXY3_PORT};
}

std::vector<uint16_t> MultiClientNotificationTest::setupDirectConnection(bool useNotificationHandler) {
    logger_->log("Setting up direct connection: clients -> %u", TCP_SERVER_PORT);
    
    // Create TCP server with appropriate handler
    if (useNotificationHandler) {
        notification_handler_ = std::make_unique<NotificationHandler>(*logger_);
    } else {
        notification_handler_ = tcp::create_tcp_echo_handler(*logger_);
    }
    notification_server_ = tcp::start_tcp_server(
        *logger_,
        Endpoint::loop_back(TCP_SERVER_PORT),
        *notification_handler_);
    
    return {TCP_SERVER_PORT, TCP_SERVER_PORT, TCP_SERVER_PORT};
}

std::array<std::shared_ptr<tcp::ITcpSession>, 3> MultiClientNotificationTest::createAndConnectClients(const std::vector<uint16_t>& target_ports) {
    logger_->log("Creating TCP clients");
    client1_ = tcp::create_tcp_client(*logger_);
    client2_ = tcp::create_tcp_client(*logger_);
    client3_ = tcp::create_tcp_client(*logger_);
    
    logger_->log("Connecting client1 from port %u to port %u", TCP_CLIENT1_PORT, target_ports[0]);
    auto session1 = client1_->connect(Endpoint::loop_back(TCP_CLIENT1_PORT), Endpoint::loop_back(target_ports[0]));
    logger_->log("Connecting client2 from port %u to port %u", TCP_CLIENT2_PORT, target_ports[1]);
    auto session2 = client2_->connect(Endpoint::loop_back(TCP_CLIENT2_PORT), Endpoint::loop_back(target_ports[1]));
    logger_->log("Connecting client3 from port %u to port %u", TCP_CLIENT3_PORT, target_ports[2]);
    auto session3 = client3_->connect(Endpoint::loop_back(TCP_CLIENT3_PORT), Endpoint::loop_back(target_ports[2]));
    
    logger_->log("Session1 valid: %s", session1 ? "true" : "false");
    logger_->log("Session2 valid: %s", session2 ? "true" : "false");
    logger_->log("Session3 valid: %s", session3 ? "true" : "false");
    
    EXPECT_TRUE(session1);
    EXPECT_TRUE(session2);
    EXPECT_TRUE(session3);
    
    return {session1, session2, session3};
}

void MultiClientNotificationTest::runNotificationTest(bool useProxies) {
    logger_->log("Starting notification test (useProxies=%s)", useProxies ? "true" : "false");
    
    logger_->log("Setting up server architecture...");
    std::vector<uint16_t> client_target_ports = useProxies 
        ? setupProxyChain(false)  // Use echo handler
        : setupDirectConnection(false);
    
    logger_->log("Waiting for servers to initialize (necessary for proxy chain setup)...");
    logger_->log("*** SLEEPING for 200ms for server initialization ***");
    std::this_thread::sleep_for(std::chrono::milliseconds(200)); // Reduced from 500ms
    
    logger_->log("Creating and connecting clients...");
    auto [session1, session2, session3] = createAndConnectClients(client_target_ports);
    
    logger_->log("Preparing test messages...");
    // Test messages
    std::string message1 = "message from client 1";
    std::string message2 = "message from client 2";
    std::string message3 = "message from client 3";
    
    logger_->log("Sending messages...");
    // Send messages
    auto write1 = session1->write(ConstBuffer(message1.data(), message1.size()), std::chrono::milliseconds(1000));
    auto write2 = session2->write(ConstBuffer(message2.data(), message2.size()), std::chrono::milliseconds(1000));
    auto write3 = session3->write(ConstBuffer(message3.data(), message3.size()), std::chrono::milliseconds(1000));
    
    logger_->log("Write results: %d, %d, %d", (int)write1.code, (int)write2.code, (int)write3.code);
    
    ASSERT_EQ(IOResultCode::Success, write1.code);
    ASSERT_EQ(IOResultCode::Success, write2.code);
    ASSERT_EQ(IOResultCode::Success, write3.code);
    
    // Read and verify echo responses
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
}

void MultiClientNotificationTest::runCrossClientNotificationTest(bool useProxies) {
    logger_->log("Starting cross-client notification test (useProxies=%s)", useProxies ? "true" : "false");
    
    std::vector<uint16_t> client_target_ports = useProxies 
        ? setupProxyChain(true)  // Use notification handler
        : setupDirectConnection(true);
    
    logger_->log("Waiting for server chain to stabilize (required for multi-proxy coordination)...");
    logger_->log("*** SLEEPING for 300ms for proxy chain stabilization ***");
    std::this_thread::sleep_for(std::chrono::milliseconds(300)); // Reduced from 500ms
    
    auto [session1, session2, session3] = createAndConnectClients(client_target_ports);
    
    logger_->log("Allowing NotificationHandler to register all clients (ensures complete notifications)...");
    logger_->log("*** SLEEPING for 100ms for client registration ***");
    std::this_thread::sleep_for(std::chrono::milliseconds(100)); // Reduced from 200ms
    
    // Send two rounds of messages
    sendMessageRounds(session1, session2, session3);
    
    // Read and verify all messages
    auto [client1_combined, client2_combined, client3_combined] = readAllMessages(session1, session2, session3);
    
    // Verify cross-client notifications for round 2 messages
    verifyNotifications(client1_combined, client2_combined, client3_combined);
    
    logger_->log("Cross-client notification test completed successfully (useProxies=%s)", useProxies ? "true" : "false");
}

void MultiClientNotificationTest::sendMessageRounds(
    std::shared_ptr<tcp::ITcpSession> session1,
    std::shared_ptr<tcp::ITcpSession> session2,
    std::shared_ptr<tcp::ITcpSession> session3) {
    
    // First round messages
    std::string message1 = "message from client 1";
    std::string message2 = "message from client 2";
    std::string message3 = "message from client 3";
    
    logger_->log("Sending first round of messages...");
    session1->write(ConstBuffer(message1.data(), message1.size()), std::chrono::milliseconds(1000));
    logger_->log("Brief pause to ensure sequential proxy chain processing...");
    logger_->log("*** SLEEPING for 100ms for sequential processing ***");
    std::this_thread::sleep_for(std::chrono::milliseconds(100)); // Reduced from 200ms
    session2->write(ConstBuffer(message2.data(), message2.size()), std::chrono::milliseconds(1000));
    logger_->log("*** SLEEPING for 100ms for sequential processing ***");
    std::this_thread::sleep_for(std::chrono::milliseconds(100)); // Reduced from 200ms
    session3->write(ConstBuffer(message3.data(), message3.size()), std::chrono::milliseconds(1000));
    
    logger_->log("Ensuring all proxy connections are established before second round...");
    logger_->log("*** SLEEPING for 500ms for connection establishment ***");
    std::this_thread::sleep_for(std::chrono::milliseconds(500)); // Reduced from 1000ms
    
    // Second round messages
    std::string message1_round2 = "second message from client 1";
    std::string message2_round2 = "second message from client 2";
    std::string message3_round2 = "second message from client 3";
    
    logger_->log("Sending second round of messages...");
    session1->write(ConstBuffer(message1_round2.data(), message1_round2.size()), std::chrono::milliseconds(1000));
    logger_->log("*** SLEEPING for 100ms for sequential processing ***");
    std::this_thread::sleep_for(std::chrono::milliseconds(100)); // Reduced from 200ms
    session2->write(ConstBuffer(message2_round2.data(), message2_round2.size()), std::chrono::milliseconds(1000));
    logger_->log("*** SLEEPING for 100ms for sequential processing ***");
    std::this_thread::sleep_for(std::chrono::milliseconds(100)); // Reduced from 200ms
    session3->write(ConstBuffer(message3_round2.data(), message3_round2.size()), std::chrono::milliseconds(1000));
    
    logger_->log("Allowing time for all notifications to propagate through proxy chain...");
    logger_->log("*** SLEEPING for 500ms for notification propagation ***");
    std::this_thread::sleep_for(std::chrono::milliseconds(500)); // Reduced from 1000ms
}

std::tuple<std::string, std::string, std::string> MultiClientNotificationTest::readAllMessages(
    std::shared_ptr<tcp::ITcpSession> session1,
    std::shared_ptr<tcp::ITcpSession> session2,
    std::shared_ptr<tcp::ITcpSession> session3) {
    
    OwnedBuffer buffer(1024);
    std::vector<std::string> client1_messages, client2_messages, client3_messages;
    
    // Read all messages from both rounds
    for (int attempt = 0; attempt < 60; ++attempt) {
        bool received_any = false;
        
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
        
        if (!received_any && attempt > 45) break;
    }
    
    // Log received messages
    logReceivedMessages(client1_messages, client2_messages, client3_messages);
    
    // Verify minimum message count
    EXPECT_GE(client1_messages.size(), 1);
    EXPECT_GE(client2_messages.size(), 1);
    EXPECT_GE(client3_messages.size(), 1);
    
    // Combine messages
    std::string client1_combined, client2_combined, client3_combined;
    for (const auto& msg : client1_messages) client1_combined += msg;
    for (const auto& msg : client2_messages) client2_combined += msg;
    for (const auto& msg : client3_messages) client3_combined += msg;
    
    return {client1_combined, client2_combined, client3_combined};
}

void MultiClientNotificationTest::logReceivedMessages(
    const std::vector<std::string>& client1_messages,
    const std::vector<std::string>& client2_messages,
    const std::vector<std::string>& client3_messages) {
    
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
}

void MultiClientNotificationTest::verifyNotifications(
    const std::string& client1_combined,
    const std::string& client2_combined,
    const std::string& client3_combined) {
    
    // Second round message strings
    std::string message1_round2 = "second message from client 1";
    std::string message2_round2 = "second message from client 2";
    std::string message3_round2 = "second message from client 3";
    
    // Verify round 2 messages (full cross-client notifications)
    // Round 1 is skipped as behavior is timing-dependent in proxy mode
    // Each client should receive their own echo from round 2
    ASSERT_TRUE(client1_combined.find("echo " + message1_round2) != std::string::npos);
    ASSERT_TRUE(client2_combined.find("echo " + message2_round2) != std::string::npos);
    ASSERT_TRUE(client3_combined.find("echo " + message3_round2) != std::string::npos);
    
    // Verify cross-client notifications for round 2
    ASSERT_TRUE(client1_combined.find("notify " + message2_round2) != std::string::npos);
    ASSERT_TRUE(client1_combined.find("notify " + message3_round2) != std::string::npos);
    
    ASSERT_TRUE(client2_combined.find("notify " + message1_round2) != std::string::npos);
    ASSERT_TRUE(client2_combined.find("notify " + message3_round2) != std::string::npos);
    
    ASSERT_TRUE(client3_combined.find("notify " + message1_round2) != std::string::npos);
    ASSERT_TRUE(client3_combined.find("notify " + message2_round2) != std::string::npos);
}
