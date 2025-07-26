#pragma once

#include <common/server/IBackgroundServer.h>
#include <background/BackgroundRunner.h>
#include <libudp/server/IUdpClientHandler.h>
#include <liblogger/ILogger.h>

#include <gtest/gtest.h>

class UdpClientServerTest : public testing::Test {
protected:
    void runTest() const;

    void SetUp() override;
    void TearDown() override;

private:
    std::unique_ptr<logger::ILogger> logger_;
    std::unique_ptr<udp::IUdpClientHandler> client_handler_;
    std::unique_ptr<IBackgroundServer> server_;
};
