#pragma once

#include <gtest/gtest.h>

#include <common/server/IBackgroundServer.h>
#include <liblogger/ILogger.h>
#include <libtcp/client/ITcpClient.h>
#include <libtcp/server/ITcpClientHandler.h>
#include <libtcp/ITcpSession.h>

#include <memory>
#include <string>
#include <vector>

// Test fixture for verifying large message handling through proxy chain
class LargeMessageTest : public testing::Test {
public:
    void SetUp() override;
    void TearDown() override;

protected:
    // Test implementations
    void runLargeMessageTest(bool useProxies, size_t messageSize);
    
    // Test constants
    static constexpr size_t SMALL_MESSAGE_SIZE = 1024;
    static constexpr size_t LARGE_MESSAGE_SIZE = 10240;
    static constexpr size_t VERY_LARGE_MESSAGE_SIZE = 50000;
    static constexpr auto WRITE_TIMEOUT = std::chrono::milliseconds(5000);
    static constexpr auto READ_TIMEOUT = std::chrono::milliseconds(2000);
    static constexpr auto TEARDOWN_SLEEP = std::chrono::milliseconds(100);
    static constexpr auto CONNECTION_SLEEP = std::chrono::milliseconds(200);
    
private:
    // Setup methods
    void stopAllServers();
    std::vector<uint16_t> setupProxyChain();
    std::vector<uint16_t> setupDirectConnection();
    std::shared_ptr<tcp::ITcpSession> createAndConnectClient(uint16_t target_port);
    
    // Message generation and verification
    std::string generateLargeMessage(size_t size) const;
    bool verifyMessageIntegrity(const std::string& original, const std::string& received);
    
    // Extracted methods from runLargeMessageTest
    std::vector<uint16_t> setupTestInfrastructure(bool useProxies);
    void sendLargeMessage(std::shared_ptr<tcp::ITcpSession> session, const std::string& message);
    std::string receiveAndVerifyMessage(std::shared_ptr<tcp::ITcpSession> session, const std::string& originalMessage);
    std::string handleMessageFragmentation(std::shared_ptr<tcp::ITcpSession> session, size_t expectedSize);
    
    // Port constants
    static constexpr uint16_t TCP_CLIENT_PORT = 18000;
    static constexpr uint16_t TCP_CLIENT_PROXY_PORT = 18001;
    static constexpr uint16_t TCP_SERVER_PORT = 18002;
    static constexpr uint16_t TCP_SERVER_PROXY_PORT = 18003;
    
    // Member variables
    std::unique_ptr<logger::ILogger> logger_;
    std::unique_ptr<tcp::ITcpClientHandler> echoHandler_;
    std::unique_ptr<IBackgroundServer> echoServer_;
    std::unique_ptr<IBackgroundServer> clientProxy_;
    std::unique_ptr<IBackgroundServer> serverProxy_;
    
    // TCP client (must stay alive during tests)
    std::unique_ptr<tcp::ITcpClient> client_;
}; 