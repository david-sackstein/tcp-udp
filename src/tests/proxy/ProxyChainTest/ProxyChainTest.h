#pragma once

#include <background/BackgroundRunner.h>
#include <libtcp/Exports.h>
#include <libtcpserverproxy/Exports.h>

#include <gtest/gtest.h>

class ProxyChainTest : public testing::Test {
public:
    void SetUp() override;
    void TearDown() override;

protected:
    // Test constants
    static constexpr uint16_t TCP_CLIENT_PORT = 15000;
    static constexpr uint16_t TCP_SERVER_PROXY_PORT = 15001;
    static constexpr uint16_t TCP_CLIENT_PROXY_PORT = 15002;
    static constexpr uint16_t TCP_SERVER_PORT = 15003;

    static constexpr size_t BUFFER_SIZE = 1024;
    static constexpr int STRESS_TEST_REQUESTS = 10000;
    static constexpr int PROGRESS_INDICATOR_INTERVAL = 1000;

    std::unique_ptr<logger::ILogger> logger_;
    std::unique_ptr<tcp::ITcpClientHandler> client_handler_;
    std::unique_ptr<IBackgroundServer> serverProxy_;
    std::unique_ptr<IBackgroundServer> clientProxy_;
    std::unique_ptr<IBackgroundServer> tcp_server_;
};