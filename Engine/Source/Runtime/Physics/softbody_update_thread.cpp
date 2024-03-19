#include "softbody.h"

#include "common.h"

#include <GLFW/glfw3.h>
#include <Windows.h>
#define WIN32_LEAN_AND_MEAN

CRITICAL_SECTION cs;

DWORD WINAPI UpdateSoftBodyThread(LPVOID lpParameter) {
    gdp1::SoftBodyThreadInfo* tInfo = reinterpret_cast<gdp1::SoftBodyThreadInfo*>(lpParameter);

    double lastFrameTime = glfwGetTime();
    double elapsedTime = 0.0;

    DWORD sleepTime_ms = 1;

    //InitializeCriticalSection(&cs);

    while (tInfo->isAlive) {
        if (tInfo->keepRunning) {
            // Adjust sleep time based on actual framerate
            double currentTime = glfwGetTime();
            double deltaTime = currentTime - lastFrameTime;
            lastFrameTime = currentTime;

            elapsedTime += deltaTime;

            //EnterCriticalSection(&cs);

            if (elapsedTime >= tInfo->timeStep) {
                // Reset the elapsed time
                elapsedTime = 0.0;

                // Update the object
                tInfo->body->Update(tInfo->timeStep);
            }

            //LeaveCriticalSection(&cs);

            // Maybe something small like 1ms delay?
            Sleep(tInfo->sleepTime);

        }
    }

    delete tInfo;

    return 0;
}