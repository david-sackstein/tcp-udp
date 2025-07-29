#pragma once

#include <background/BackgroundRunner.h>
#include <libtcp/Exports.h>
#include <libtcpserverproxy/Exports.h>

#include <gtest/gtest.h>
#include <array>
#include <memory>
#include <vector>
#include <tuple>

class MultiClientNotificationTest : public testing::Test {
public:
    void SetUp() override;
    void TearDown() override;

protected:
    // Main test implementations
    void runNotificationTest(bool useProxies);
    void runCrossClientNotificationTest(bool useProxies);

private:
    // Setup methods
    void stopAllServers();
    std::vector<uint16_t> setupProxyChain(bool useNotificationHandler);
    std::vector<uint16_t> setupDirectConnection(bool useNotificationHandler);
    std::array<std::shared_ptr<tcp::ITcpSession>, 3> createAndConnectClients(const std::vector<uint16_t>& target_ports);
    
    // Test execution methods
    void sendMessageRounds(
        std::shared_ptr<tcp::ITcpSession> session1,
        std::shared_ptr<tcp::ITcpSession> session2,
        std::shared_ptr<tcp::ITcpSession> session3);
    
    std::tuple<std::string, std::string, std::string> readAllMessages(
        std::shared_ptr<tcp::ITcpSession> session1,
        std::shared_ptr<tcp::ITcpSession> session2,
        std::shared_ptr<tcp::ITcpSession> session3);
    
    void logReceivedMessages(
        const std::vector<std::string>& client1_messages,
        const std::vector<std::string>& client2_messages,
        const std::vector<std::string>& client3_messages) const;
    
    void verifyNotifications(
        const std::string& client1_combined,
        const std::string& client2_combined,
        const std::string& client3_combined) const;

    // Port constants
    const uint16_t TCP_CLIENT1_PORT = 16000;
    const uint16_t TCP_CLIENT2_PORT = 16001;
    const uint16_t TCP_CLIENT3_PORT = 16002;
    const uint16_t TCP_CLIENT_PROXY_PORT = 17002;
    const uint16_t TCP_SERVER_PORT = 17003;
    const uint16_t TCP_SERVER_PROXY1_PORT = 17001;
    const uint16_t TCP_SERVER_PROXY2_PORT = 17004;
    const uint16_t TCP_SERVER_PROXY3_PORT = 17005;
    
    // Member variables
    std::unique_ptr<logger::ILogger> logger_;
    std::unique_ptr<tcp::ITcpClientHandler> notification_handler_;
    
    // Server and proxy instances
    std::unique_ptr<IBackgroundServer> notification_server_;
    std::unique_ptr<IBackgroundServer> clientProxy_;
    std::unique_ptr<IBackgroundServer> serverProxy1_;
    std::unique_ptr<IBackgroundServer> serverProxy2_;
    std::unique_ptr<IBackgroundServer> serverProxy3_;
    
    // TCP clients (must stay alive during tests)
    std::unique_ptr<tcp::ITcpClient> client1_;
    std::unique_ptr<tcp::ITcpClient> client2_;
    std::unique_ptr<tcp::ITcpClient> client3_;
}; 