#pragma once

#include "BackgroundServer.h"

#include <functional>
#include <future>

inline std::unique_ptr<IBackgroundServer> startServer(std::function<std::unique_ptr<IBlockingServer>()> factory,
    std::function<void(IBlockingServer&)> runner) {
    std::promise<std::shared_ptr<IBlockingServer>> promise;
    auto future = promise.get_future();

    std::thread t([&]() {
        // create and run in the same thread (ACE requires it)
        auto server = std::shared_ptr<IBlockingServer>(factory());
        promise.set_value(server);
        runner(*server);
    });

    return std::make_unique<BackgroundServer>(std::move(t), future.get());
}
