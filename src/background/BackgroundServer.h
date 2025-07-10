#pragma once

#include <common/server/IBackgroundServer.h>
#include <common/server/IBlockingServer.h>

#include <thread>
#include <memory>
#include <utility>

class BackgroundServer : public IBackgroundServer {
public:
    std::thread thread_;
    std::shared_ptr<IBlockingServer> server_;

    BackgroundServer(std::thread t, std::shared_ptr<IBlockingServer> s) :
        thread_(std::move(t)),
        server_(std::move(s))
    {
    }

    [[nodiscard]] const Endpoint& get_local_endpoint() const override {
        return server_->get_local_endpoint();
    }

    void stop() override {
        server_->stop();
        thread_.join();
    }
};
