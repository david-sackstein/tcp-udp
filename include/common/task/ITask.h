#pragma once

class ITask {
public:
    virtual ~ITask() = default;
    // Blocks until the task is fully stopped
    virtual void stop() = 0;
};
