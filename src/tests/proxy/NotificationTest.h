#pragma once

#include <background/BackgroundRunner.h>
#include <libtcp/Exports.h>
#include <libtcpserverproxy/Exports.h>
#include <libtcpclientproxy/Exports.h>

#include <gtest/gtest.h>

class MultiClientNotificationTest : public testing::Test {
public:
    void SetUp() override;
    void TearDown() override;

protected:
    // Common test implementation that can run with or without proxies
    void runNotificationTest(bool useProxies);
    
    // Test implementation for cross-client notifications
    void runCrossClientNotificationTest(bool useProxies);

    const uint16_t TCP_CLIENT1_PORT = 16000;
    const uint16_t TCP_CLIENT2_PORT = 16001;
    const uint16_t TCP_CLIENT3_PORT = 16002;
    
    // For direct connection: multiple TCP servers
    const uint16_t TCP_SERVER1_PORT = 17001;
    const uint16_t TCP_SERVER2_PORT = 17004;
    const uint16_t TCP_SERVER3_PORT = 17005;
    
    // For proxy connection: multiple server proxies + one client proxy + one TCP server
    const uint16_t TCP_SERVER_PROXY1_PORT = 17001;
    const uint16_t TCP_SERVER_PROXY2_PORT = 17004;
    const uint16_t TCP_SERVER_PROXY3_PORT = 17005;
    const uint16_t TCP_CLIENT_PROXY_PORT = 17002;
    const uint16_t TCP_SERVER_PORT = 17003;
    
    std::unique_ptr<logger::ILogger> logger_;
    
    // Notification handlers (must be kept alive)
    std::unique_ptr<tcp::ITcpClientHandler> notification_handler_;
    std::unique_ptr<tcp::ITcpClientHandler> notification_handler1_;
    std::unique_ptr<tcp::ITcpClientHandler> notification_handler2_;
    std::unique_ptr<tcp::ITcpClientHandler> notification_handler3_;
    
    // For direct connection: multiple notification servers
    std::unique_ptr<IBackgroundServer> notification_server1_;
    std::unique_ptr<IBackgroundServer> notification_server2_;
    std::unique_ptr<IBackgroundServer> notification_server3_;
    
    // For proxy connection: multiple server proxies + one client proxy + one notification server
    std::unique_ptr<IBackgroundServer> serverProxy1_;
    std::unique_ptr<IBackgroundServer> serverProxy2_;
    std::unique_ptr<IBackgroundServer> serverProxy3_;
    std::unique_ptr<IBackgroundServer> clientProxy_;
    std::unique_ptr<IBackgroundServer> notification_server_;
}; 