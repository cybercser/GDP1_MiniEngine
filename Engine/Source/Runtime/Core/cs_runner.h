#pragma once

#include <queue>
#include <Windows.h>

namespace gdp1 {

class CSRunner {
public:
    CSRunner()
        : num_readers(0)
        , writing(false) {
        InitializeCriticalSection(&cs);
        InitializeConditionVariable(&read_cv);
        InitializeConditionVariable(&write_cv);
    }

    ~CSRunner() { DeleteCriticalSection(&cs); }

    void StartReadLock() {
        EnterCriticalSection(&cs);
        while (writing) {
            SleepConditionVariableCS(&read_cv, &cs, INFINITE);
        }
        InterlockedIncrement(&num_readers);
        LeaveCriticalSection(&cs);
    }

    void EndReadLock() {
        EnterCriticalSection(&cs);
        InterlockedDecrement(&num_readers);
        if (num_readers == 0) {
            WakeAllConditionVariable(&write_cv);
        }
        LeaveCriticalSection(&cs);
    }

    void StartWriteLock() {
        EnterCriticalSection(&cs);
        while (num_readers > 0 || writing) {
            SleepConditionVariableCS(&write_cv, &cs, INFINITE);
        }
        writing = true;
        LeaveCriticalSection(&cs);
    }

    void EndWriteLock() {
        EnterCriticalSection(&cs);
        writing = false;
        WakeAllConditionVariable(&read_cv);
        WakeAllConditionVariable(&write_cv);
        LeaveCriticalSection(&cs);
    }
private:
    CRITICAL_SECTION cs;
    CONDITION_VARIABLE read_cv;
    CONDITION_VARIABLE write_cv;

    std::queue<std::function<void()>> tasks;

    LONG num_readers;
    bool writing;
};

}  // namespace gdp1
