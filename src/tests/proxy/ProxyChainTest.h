#pragma once

#include <background/BackgroundRunner.h>
#include <libtcp/Exports.h>
#include <libtcpserverproxy/Exports.h>
#include <libtcpclientproxy/Exports.h>

#include <gtest/gtest.h>

class ProxyChainTest : public ::testing::Test {
public:
    void SetUp() override;
    void TearDown() override;

protected:
    const uint16_t TCP_CLIENT_PORT = 15000;
    const uint16_t TCP_SERVER_PROXY_PORT = 15001;
    const uint16_t TCP_CLIENT_PROXY_PORT = 15002;
    const uint16_t TCP_SERVER_PORT = 15003;

    std::unique_ptr<tcp::ITcpClientHandler> client_handler_;
    std::unique_ptr<IBackgroundServer> serverProxy_;
    std::unique_ptr<IBackgroundServer> clientProxy_;
    std::unique_ptr<IBackgroundServer> tcp_server_;
};