#include "ProxyChainTest.h"

#include <common/OwnedBuffer.h>
#include <libtcp/Exports.h>
#include <libtcpserverproxy/Exports.h>
#include <libtcpclientproxy/Exports.h>
#include <libtcp/client/ITcpClient.h>
#include <string>

static std::chrono::milliseconds block = std::chrono::milliseconds::max();

TEST_F(ProxyChainTest, EndToEnd) {
    auto client = tcp::create_tcp_client();

    std::shared_ptr<tcp::ITcpSession> session = client->connect(
            Endpoint::loop_back(TCP_CLIENT_PORT),
            Endpoint::loop_back(TCP_SERVER_PROXY_PORT));

    ASSERT_TRUE(session);

    auto msg = "hello";
    auto size = session->write(ConstBuffer(msg, 5), block);
    (void) size;

    OwnedBuffer buffer(1024);

    const auto result = session->read(buffer.view(), block);

    ASSERT_EQ(IOResultCode::Success, result.code);
    std::string expected_response = "echo " + std::string(msg, 5);
    ASSERT_STREQ(buffer.view().data, expected_response.c_str());

    client->disconnect();
}

void ProxyChainTest::SetUp() {

    client_handler_ = tcp::create_tcp_echo_handler();

    serverProxy_ = server_proxy::start_tcp_server_proxy(
            Endpoint::loop_back(TCP_SERVER_PROXY_PORT),
            Endpoint::loop_back(TCP_CLIENT_PROXY_PORT));

    clientProxy_ = client_proxy::start_tcp_client_proxy(
            Endpoint::loop_back(TCP_CLIENT_PROXY_PORT),
            Endpoint::loop_back(TCP_SERVER_PORT));

    tcp_server_ = start_tcp_server(
            Endpoint::loop_back(TCP_SERVER_PORT),
            *client_handler_);

    // Wait for servers to be ready
    std::this_thread::sleep_for(std::chrono::milliseconds(100));
}

void ProxyChainTest::TearDown() {
    tcp_server_->stop();
    clientProxy_->stop();
    serverProxy_->stop();
}

