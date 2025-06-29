#include "UdpClientServerTest.h"
#include "common/OwnedBuffer.h"

#include <libudp/Exports.h>

#include <iostream>

TEST_F(UdpClientServerTest, ClientSendsAndReceivesEcho) {
    runTest();
}

void UdpClientServerTest::runTest() const {
    const auto server_endpoint = server_->get_local_endpoint();

    auto client = udp::create_udp_client(Endpoint::any_loop_back()); // Bind to ephemeral port
    ASSERT_NE(client, nullptr);

    const char *msg = "hello udp server";
    const size_t msg_len = strlen(msg);

    const bool sent = client->send_to(server_endpoint, ConstBuffer(msg, msg_len));
    ASSERT_TRUE(sent);

    OwnedBuffer buffer(1024);

    Endpoint sender;

    // Wait and receive echoed message
    ssize_t received = client->receive_from(buffer.view(), sender);
    ASSERT_GT(received, 0);

    // Verify size matches (original message + "echo " prefix)
    std::string expected_response = "echo " + std::string(msg, msg_len);
    ASSERT_EQ(static_cast<size_t>(received), expected_response.size());

    // Verify content matches exactly
    ASSERT_EQ(std::string(buffer.view().data, received), expected_response);
}

void UdpClientServerTest::SetUp() {
    client_handler_ = udp::create_udp_echo_handler();
    server_ = udp::start_udp_server(Endpoint::any_loop_back(), *client_handler_);
}

void UdpClientServerTest::TearDown() {
    server_->stop();
}
