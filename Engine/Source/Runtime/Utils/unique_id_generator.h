#pragma once

#include <iostream>
#include <atomic>

namespace gdp1 {

class UniqueId {
private:
    static std::atomic<int> nextId;

public:
    static int GenerateId() { return nextId++; }
};

}  // namespace gdp1

// Initialize the static member outside the class definition
std::atomic<int> gdp1::UniqueId::nextId(1);