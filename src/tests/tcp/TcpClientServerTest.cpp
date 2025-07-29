#include "TcpClientServerTest.h"

#include <liblogger/Exports.h>

#include "common/OwnedBuffer.h"

#include <libtcp/Exports.h>
#include <libtcp/client/ITcpClient.h>

static std::chrono::milliseconds block = std::chrono::milliseconds::max();

TEST_F(TcpClientServerTest, ClientConnectsAndReceivesEcho) {
    runTest();
}

void TcpClientServerTest::runTest() {
    const Endpoint& endpoint = server_->get_local_endpoint();

    const auto client = tcp::create_tcp_client(*logger_);
    const auto session = client->connect(Endpoint::any_loop_back(), endpoint);
    ASSERT_NE(session, nullptr);

    const auto msg = std::string("hello server");
    const auto write_result = session->write({msg.data(), msg.size()}, block);
    ASSERT_EQ(write_result.count, msg.size());

    OwnedBuffer buffer(1024);

    const auto read_result = session->read(buffer.view(), block);
    const std::string expected_response = "echo [" + msg + "]";
    ASSERT_EQ(read_result.count, expected_response.size());
    ASSERT_STREQ(buffer.view().data, expected_response.data());

    session->close();
    client->disconnect();
}

void TcpClientServerTest::SetUp() {
    logger_ = logger::create_console_logger(logger::LogLevel::ERROR);
    client_handler_ = tcp::create_tcp_echo_handler(*logger_);
    server_ = tcp::start_tcp_server(*logger_, Endpoint::any_loop_back(), *client_handler_);
}

void TcpClientServerTest::TearDown() {
    server_->stop();
}
