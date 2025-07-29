#pragma once

#include <background/BackgroundRunner.h>
#include <libtcp/Exports.h>
#include <libtcpserverproxy/Exports.h>

#include <gtest/gtest.h>
#include <array>
#include <memory>
#include <tuple>
#include <vector>

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
    void sendMessageRounds(const std::shared_ptr<tcp::ITcpSession>& session1,
        const std::shared_ptr<tcp::ITcpSession>& session2,
        const std::shared_ptr<tcp::ITcpSession>& session3) const;

    std::tuple<std::string, std::string, std::string> readAllMessages(const std::shared_ptr<tcp::ITcpSession>& session1,
        const std::shared_ptr<tcp::ITcpSession>& session2,
        const std::shared_ptr<tcp::ITcpSession>& session3);

    static std::vector<std::string> readMessagesFromSession(std::shared_ptr<tcp::ITcpSession> session);

    static std::vector<std::string> processReceivedMessages(const std::shared_ptr<tcp::ITcpSession>& session);

    static std::string combineMessages(const std::vector<std::string>& messages);

    std::vector<uint16_t> setupNotificationTest(bool useProxies);
    void executeNotificationRound(std::shared_ptr<tcp::ITcpSession> session1,
        std::shared_ptr<tcp::ITcpSession> session2,
        std::shared_ptr<tcp::ITcpSession> session3);

    void verifyCrossClientNotifications(std::shared_ptr<tcp::ITcpSession> session1,
        std::shared_ptr<tcp::ITcpSession> session2,
        std::shared_ptr<tcp::ITcpSession> session3);

    void logReceivedMessages(const std::vector<std::string>& client1_messages,
        const std::vector<std::string>& client2_messages,
        const std::vector<std::string>& client3_messages) const;

    static void verifyNotifications(const std::string& client1_combined,
        const std::string& client2_combined,
        const std::string& client3_combined);

    // Port constants
    static constexpr uint16_t TCP_CLIENT1_PORT = 16000;
    static constexpr uint16_t TCP_CLIENT2_PORT = 16001;
    static constexpr uint16_t TCP_CLIENT3_PORT = 16002;
    static constexpr uint16_t TCP_CLIENT_PROXY_PORT = 17002;
    static constexpr uint16_t TCP_SERVER_PORT = 17003;
    static constexpr uint16_t TCP_SERVER_PROXY1_PORT = 17001;
    static constexpr uint16_t TCP_SERVER_PROXY2_PORT = 17004;
    static constexpr uint16_t TCP_SERVER_PROXY3_PORT = 17005;

    // Test constants
    static constexpr size_t BUFFER_SIZE = 1024;
    static constexpr std::chrono::milliseconds WRITE_TIMEOUT{1000};
    static constexpr std::chrono::milliseconds READ_TIMEOUT{2000};
    static constexpr std::chrono::milliseconds SHORT_READ_TIMEOUT{50};
    static constexpr std::chrono::milliseconds TEARDOWN_SLEEP{100};
    static constexpr std::chrono::milliseconds SEQUENTIAL_SLEEP{100};
    static constexpr std::chrono::milliseconds CONNECTION_SLEEP{200};
    static constexpr std::chrono::milliseconds STABILIZATION_SLEEP{300};
    static constexpr std::chrono::milliseconds NOTIFICATION_SLEEP{500};
    static constexpr int MAX_READ_ATTEMPTS = 60;
    static constexpr int READ_BREAK_THRESHOLD = 45;

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