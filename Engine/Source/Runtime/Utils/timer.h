#pragma once

#include <iostream>
#include <chrono>
#include <string>

#include "Core/logger.h"

class GTimer {
public:
    GTimer(std::string tag)
        : start(std::chrono::high_resolution_clock::now()), tag(tag) {}

    ~GTimer() {
        auto end = std::chrono::high_resolution_clock::now();
        auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(end - start);
        LOG_INFO("{0}: {1} millis", tag, duration.count());
    }

private:
    std::chrono::time_point<std::chrono::high_resolution_clock> start;
    std::string tag;
};