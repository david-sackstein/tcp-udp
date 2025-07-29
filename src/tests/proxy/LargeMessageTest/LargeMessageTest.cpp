#include "LargeMessageTest.h"
#include "SimpleEchoHandler.h"

#include <libtcp/Exports.h>
#include <libtcpclientproxy/Exports.h>
#include <libtcpserverproxy/Exports.h>
#include <libudp/Exports.h>
#include <liblogger/Exports.h>

#include <common/Endpoint.h>
#include <common/OwnedBuffer.h>
#include <libacetools/IOResultCode.h>

#include <chrono>
#include <thread>
#include <sstream>
#include <iomanip>
#include <algorithm>

// Test with direct connection (no proxies) - small message
TEST_F(LargeMessageTest, DirectConnection_SmallMessage) {
    runLargeMessageTest(false, 1024);
}

// Test with direct connection (no proxies) - large message
TEST_F(LargeMessageTest, DirectConnection_LargeMessage) {
    runLargeMessageTest(false, 10240);
}

// Test with proxy chain - small message
TEST_F(LargeMessageTest, ProxyChain_SmallMessage) {
    runLargeMessageTest(true, 1024);
}

// Test with proxy chain - large message
TEST_F(LargeMessageTest, ProxyChain_LargeMessage) {
    runLargeMessageTest(true, 10240);
}

// Test with proxy chain - very large message (multiple UDP packets guaranteed)
TEST_F(LargeMessageTest, ProxyChain_VeryLargeMessage) {
    runLargeMessageTest(true, 50000);
}

void LargeMessageTest::SetUp() {
    logger_ = logger::create_console_logger(logger::LogLevel::ERROR);
}

void LargeMessageTest::TearDown() {
    stopAllServers();
    logger_->log(logger::LogLevel::INFO, "TearDown: Allowing time for graceful cleanup...");
    logger_->log(logger::LogLevel::INFO, "SLEEPING for 100ms for graceful cleanup");
    std::this_thread::sleep_for(std::chrono::milliseconds(100));
}

void LargeMessageTest::stopAllServers() {
    logger_->log(logger::LogLevel::INFO, "TearDown: Stopping all servers and clients");
    
    if (client_) {
        logger_->log(logger::LogLevel::INFO, "TearDown: Disconnecting client");
        client_->disconnect();
        client_.reset();
    }
    
    if (serverProxy_) {
        serverProxy_->stop();
        serverProxy_.reset();
    }
    
    if (clientProxy_) {
        clientProxy_->stop();
        clientProxy_.reset();
    }
    
    if (echoServer_) {
        echoServer_->stop();
        echoServer_.reset();
    }
}

std::vector<uint16_t> LargeMessageTest::setupProxyChain() {
    logger_->log(logger::LogLevel::INFO, "Setting up proxy chain: client -> server_proxy -> client_proxy -> echo_server");
    
    // Create echo server with simple echo handler
    echoHandler_ = std::make_unique<SimpleEchoHandler>(*logger_);
    echoServer_ = tcp::start_tcp_server(*logger_, Endpoint::loop_back(TCP_SERVER_PORT), *echoHandler_);
    
    // Create client proxy
    clientProxy_ = client_proxy::start_tcp_client_proxy(
        *logger_,
        Endpoint::loop_back(TCP_CLIENT_PROXY_PORT),
        Endpoint::loop_back(TCP_SERVER_PORT)
    );
    
    // Create server proxy
    serverProxy_ = server_proxy::start_tcp_server_proxy(
        *logger_,
        Endpoint::loop_back(TCP_SERVER_PROXY_PORT),
        Endpoint::loop_back(TCP_CLIENT_PROXY_PORT)
    );
    
    logger_->log(logger::LogLevel::INFO, "Waiting for proxy chain to initialize...");
    logger_->log(logger::LogLevel::INFO, "SLEEPING for 200ms for proxy chain initialization");
    std::this_thread::sleep_for(std::chrono::milliseconds(200));
    
    return {TCP_SERVER_PROXY_PORT};
}

std::vector<uint16_t> LargeMessageTest::setupDirectConnection() {
    logger_->log(logger::LogLevel::INFO, "Setting up direct connection: client -> echo_server");
    
    // Create echo server with simple echo handler
    echoHandler_ = std::make_unique<SimpleEchoHandler>(*logger_);
    echoServer_ = tcp::start_tcp_server(*logger_, Endpoint::loop_back(TCP_SERVER_PORT), *echoHandler_);
    
    logger_->log(logger::LogLevel::INFO, "Waiting for server to initialize...");
    logger_->log(logger::LogLevel::INFO, "SLEEPING for 200ms for server initialization");
    std::this_thread::sleep_for(std::chrono::milliseconds(200));
    
    return {TCP_SERVER_PORT};
}

std::shared_ptr<tcp::ITcpSession> LargeMessageTest::createAndConnectClient(uint16_t target_port) {
    logger_->log(logger::LogLevel::INFO, "Creating TCP client");
    client_ = std::unique_ptr<tcp::ITcpClient>(tcp::create_tcp_client(*logger_));
    
    logger_->log(logger::LogLevel::INFO, "Connecting client from port %u to port %u", TCP_CLIENT_PORT, target_port);
    auto session = client_->connect(
        Endpoint::loop_back(TCP_CLIENT_PORT),
        Endpoint::loop_back(target_port)
    );
    
    if (!session) {
        throw std::runtime_error("Failed to connect client");
    }
    
    logger_->log(logger::LogLevel::INFO, "Client connected successfully");
    return session;
}

std::string LargeMessageTest::generateLargeMessage(size_t size) const {
    std::ostringstream oss;
    
    // Create a repeating pattern with sequence numbers for verification
    size_t sequence = 0;
    size_t bytesWritten = 0;
    
    while (bytesWritten < size) {
        std::string chunk = "MSG" + std::to_string(sequence) + "_";
        
        // Ensure we don't exceed the target size
        if (bytesWritten + chunk.length() > size) {
            chunk = chunk.substr(0, size - bytesWritten);
        }
        
        oss << chunk;
        bytesWritten += chunk.length();
        sequence++;
    }
    
    std::string result = oss.str();
    logger_->log(logger::LogLevel::INFO, "Generated message of size %zu bytes (requested %zu)", result.length(), size);
    return result;
}

bool LargeMessageTest::verifyMessageIntegrity(const std::string& original, const std::string& received) {
    if (original.length() != received.length()) {
        logger_->log(logger::LogLevel::ERROR, "Message length mismatch: original=%zu, received=%zu", 
                    original.length(), received.length());
        return false;
    }
    
    if (original != received) {
        logger_->log(logger::LogLevel::ERROR, "Message content mismatch");
        
        // Find first difference for debugging
        for (size_t i = 0; i < std::min(original.length(), received.length()); ++i) {
            if (original[i] != received[i]) {
                logger_->log(logger::LogLevel::ERROR, "First difference at position %zu: original='%c' received='%c'", 
                            i, original[i], received[i]);
                break;
            }
        }
        return false;
    }
    
    logger_->log(logger::LogLevel::INFO, "Message integrity verified: %zu bytes match exactly", original.length());
    return true;
}

std::vector<uint16_t> LargeMessageTest::setupTestInfrastructure(bool useProxies) {
    logger_->log(logger::LogLevel::INFO, "Setting up test infrastructure (useProxies=%s)", 
                useProxies ? "true" : "false");
    return useProxies ? setupProxyChain() : setupDirectConnection();
}

void LargeMessageTest::sendLargeMessage(std::shared_ptr<tcp::ITcpSession> session, const std::string& message) {
    logger_->log(logger::LogLevel::INFO, "Sending message of %zu bytes...", message.length());
    ConstBuffer send_buffer(message.data(), message.length());
    auto write_result = session->write(send_buffer, std::chrono::milliseconds(5000));
    EXPECT_EQ(write_result.code, IOResultCode::Success);
    EXPECT_EQ(write_result.count, message.length());
}

std::string LargeMessageTest::receiveAndVerifyMessage(std::shared_ptr<tcp::ITcpSession> session, const std::string& originalMessage) {
    logger_->log(logger::LogLevel::INFO, "Reading echo response...");
    std::string receivedMessage = handleMessageFragmentation(session, originalMessage.length());
    
    // No prefix to remove, the entire received message is the echo content
    std::string actualEchoContent = receivedMessage;
    
    // Verify message integrity
    EXPECT_TRUE(verifyMessageIntegrity(originalMessage, actualEchoContent));
    
    return actualEchoContent;
}

std::string LargeMessageTest::handleMessageFragmentation(std::shared_ptr<tcp::ITcpSession> session, size_t expectedSize) {
    OwnedBuffer receive_buffer(expectedSize * 2); // Extra space to be safe
    size_t totalReceived = 0;
    
    // Read in a loop until we get all the data (TCP may fragment)
    while (totalReceived < expectedSize) {
        size_t remaining = expectedSize - totalReceived;
        Buffer read_buffer{
            receive_buffer.view().data + totalReceived, 
            std::min(remaining, receive_buffer.view().size - totalReceived)
        };
        
        auto read_result = session->read(read_buffer, std::chrono::milliseconds(2000));
        
        if (read_result.code == IOResultCode::Timeout) {
            logger_->log(logger::LogLevel::ERROR, "Read timeout, received %zu/%zu bytes so far", totalReceived, expectedSize);
            continue;
        }
        
        EXPECT_EQ(read_result.code, IOResultCode::Success);
        EXPECT_GT(read_result.count, 0u);
        
        totalReceived += read_result.count;
        logger_->log(logger::LogLevel::INFO, "Read %zu bytes, total received: %zu/%zu", 
                    read_result.count, totalReceived, expectedSize);
    }
    
    // Construct received message
    std::string receivedMessage;
    receivedMessage.assign(receive_buffer.view().data, totalReceived);
    return receivedMessage;
}

void LargeMessageTest::runLargeMessageTest(bool useProxies, size_t messageSize) {
    logger_->log(logger::LogLevel::INFO, "Starting large message test (useProxies=%s, size=%zu)", 
                useProxies ? "true" : "false", messageSize);
    
    // Setup infrastructure
    std::vector<uint16_t> target_ports = setupTestInfrastructure(useProxies);
    
    // Connect client
    auto session = createAndConnectClient(target_ports[0]);
    ASSERT_TRUE(session != nullptr);
    
    // Generate test message
    std::string originalMessage = generateLargeMessage(messageSize);
    ASSERT_EQ(originalMessage.length(), messageSize);
    
    // Send message
    sendLargeMessage(session, originalMessage);
    
    // Receive and verify message
    receiveAndVerifyMessage(session, originalMessage);
    
    logger_->log(logger::LogLevel::INFO, "Large message test completed successfully (useProxies=%s, size=%zu)", 
                useProxies ? "true" : "false", messageSize);
} 