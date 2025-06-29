#pragma once

#include "ITask.h"

#include <atomic>
#include <thread>
#include <functional>

class RunningTask final : public ITask {
public:
    using Work = std::function<void(std::atomic<bool> &)>;

    explicit RunningTask(const Work& work) :
        cancelled_(false),
        thread_([this, work] {
            work(cancelled_);
        }) {}

    ~RunningTask() override {
        cancelled_.store(true);
        if (thread_.joinable()) {
            thread_.join();
        }
    }

    void stop() override {
        cancelled_.store(true);
        if (thread_.joinable()) {
            thread_.join();
        }
    }

private:
    std::atomic<bool> cancelled_;
    std::thread thread_;
};
