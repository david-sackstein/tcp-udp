#pragma once

#include "ITask.h"

class CompletedTask final : public ITask {
public:
    CompletedTask() = default;
    void stop() override {}
};