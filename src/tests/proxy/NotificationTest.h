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

    const uint16_t TCP_CLIENT1_PORT = 16000;
    const uint16_t TCP_CLIENT2_PORT = 16001;
    const uint16_t TCP_CLIENT3_PORT = 16002;
    const uint16_t TCP_SERVER_PROXY_PORT = 17001;
    const uint16_t TCP_CLIENT_PROXY_PORT = 17002;
    const uint16_t TCP_SERVER_PORT = 17003;
    
    std::unique_ptr<logger::ILogger> logger_;
    std::unique_ptr<IBackgroundServer> serverProxy_;
    std::unique_ptr<IBackgroundServer> clientProxy_;
    std::unique_ptr<IBackgroundServer> notification_server_;
}; 