#include "softbody.h"

#include <queue>
#include <GLFW/glfw3.h>
#include <Windows.h>
#define WIN32_LEAN_AND_MEAN

DWORD WINAPI UpdateSoftBodyThread(LPVOID lpParameter) {
    gdp1::SoftBodyThreadInfo* tInfo = reinterpret_cast<gdp1::SoftBodyThreadInfo*>(lpParameter);

    double lastFrameTime = glfwGetTime();
    double elapsedTime = 0.0;

    DWORD sleepTime_ms = 1;

    while (tInfo->isAlive) {
        if (tInfo->keepRunning) {
            // Adjust sleep time based on actual framerate
            double currentTime = glfwGetTime();
            double deltaTime = currentTime - lastFrameTime;
            lastFrameTime = currentTime;

            elapsedTime += deltaTime;

            if (elapsedTime >= tInfo->timeStep) {
                // Reset the elapsed time
                elapsedTime = 0.0;

                // Queue the update task to the buffer
                tInfo->body->Update(tInfo->timeStep);
            }

            // Maybe something small like 1ms delay?
            Sleep(tInfo->sleepTime);

        }
    }

    delete tInfo;

    return 0;
}

//DWORD WINAPI UpdateSoftBodyThread(LPVOID lpParameter) {
//    gdp1::SoftBodyThreadInfo* tInfo = reinterpret_cast<gdp1::SoftBodyThreadInfo*>(lpParameter);
//
//    double lastFrameTime = glfwGetTime();
//    double elapsedTime = 0.0;
//
//    DWORD sleepTime_ms = 1;
//
//    while (tInfo->isAlive) {
//        if (tInfo->keepRunning) {
//            // Adjust sleep time based on actual framerate
//            double currentTime = glfwGetTime();
//            double deltaTime = currentTime - lastFrameTime;
//            lastFrameTime = currentTime;
//
//            elapsedTime += deltaTime;
//
//            if (elapsedTime >= tInfo->timeStep) {
//                // Reset the elapsed time
//                elapsedTime = 0.0;
//
//                // Update the object
//
//                tInfo->body->Update(tInfo->timeStep);
//            }
//
//            // Maybe something small like 1ms delay?
//            Sleep(tInfo->sleepTime);
//        }
//    }
//
//    delete tInfo;
//
//    return 0;
//}