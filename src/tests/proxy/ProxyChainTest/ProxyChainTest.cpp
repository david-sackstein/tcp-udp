#include "ProxyChainTest.h"

#include <common/OwnedBuffer.h>
#include <liblogger/Exports.h>
#include <libtcp/Exports.h>
#include <libtcpserverproxy/Exports.h>
#include <libtcpclientproxy/Exports.h>
#include <libtcp/client/ITcpClient.h>

#include <string>
#include <sstream>

static std::chrono::milliseconds block = std::chrono::milliseconds::max();

TEST_F(ProxyChainTest, EndToEnd) {
    auto client = tcp::create_tcp_client(*logger_);

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

TEST_F(ProxyChainTest, StressTest) {
    auto client = tcp::create_tcp_client(*logger_);

    std::shared_ptr<tcp::ITcpSession> session = client->connect(
        Endpoint::loop_back(TCP_CLIENT_PORT),
        Endpoint::loop_back(TCP_SERVER_PROXY_PORT));

    ASSERT_TRUE(session);

    const int num_requests = 10000;
    OwnedBuffer buffer(1024);

    for (int i = 1; i <= num_requests; i++) {
        // Create message with incrementing number
        std::ostringstream oss;
        oss << "request_" << i;
        std::string msg = oss.str();

        // Send request
        auto write_result = session->write(ConstBuffer(msg.data(), msg.size()), block);
        ASSERT_EQ(IOResultCode::Success, write_result.code);
        ASSERT_EQ(msg.size(), write_result.count);

        // Read response
        const auto read_result = session->read(buffer.view(), block);
        ASSERT_EQ(IOResultCode::Success, read_result.code);

        // Verify response
        std::string response(buffer.view().data, read_result.count);
        std::string expected_response = "echo " + msg;
        ASSERT_EQ(expected_response, response) << "Failed on request " << i;

        // Progress indicator every 1000 requests
        if (i % 1000 == 0) {
            logger_->log(logger::LogLevel::INFO, "StressTest: Completed %d/%d requests", i, num_requests);
        }
    }

    logger_->log(logger::LogLevel::INFO, "StressTest: All %d requests completed successfully", num_requests);
    client->disconnect();
}

void ProxyChainTest::SetUp() {
    logger_ = logger::create_console_logger(logger::LogLevel::ERROR);
    client_handler_ = tcp::create_tcp_echo_handler(*logger_);

    serverProxy_ = server_proxy::start_tcp_server_proxy(
        *logger_,
        Endpoint::loop_back(TCP_SERVER_PROXY_PORT),
        Endpoint::loop_back(TCP_CLIENT_PROXY_PORT));

    clientProxy_ = client_proxy::start_tcp_client_proxy(
        *logger_,
        Endpoint::loop_back(TCP_CLIENT_PROXY_PORT),
        Endpoint::loop_back(TCP_SERVER_PORT));

    tcp_server_ = tcp::start_tcp_server(
        *logger_,
        Endpoint::loop_back(TCP_SERVER_PORT),
        *client_handler_);
}

void ProxyChainTest::TearDown() {
    tcp_server_->stop();
    clientProxy_->stop();
    serverProxy_->stop();
}
