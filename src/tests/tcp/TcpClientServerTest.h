#pragma once

#include <background/BackgroundRunner.h>
#include <common/server/IBackgroundServer.h>
#include <libtcp/server/ITcpClientHandler.h>
#include <liblogger/ILogger.h>

#include <gtest/gtest.h>

#include <memory>

class TcpClientServerTest : public ::testing::Test {
protected:
    void SetUp() override;
    void TearDown() override;
    void runTest();

private:
    static constexpr size_t BUFFER_SIZE = 1024;
    
    std::unique_ptr<tcp::ITcpClientHandler> client_handler_;
    std::unique_ptr<IBackgroundServer> server_;
    std::unique_ptr<logger::ILogger> logger_;
};
