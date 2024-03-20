#include "scene.h"
#include "model.h"

std::mutex g_Mutex;
extern CRITICAL_SECTION g_CriticalSection;

DWORD WINAPI LoadModelThread(LPVOID lpParameter) {

    gdp1::LoadModelThreadParams* params = reinterpret_cast<gdp1::LoadModelThreadParams*>(lpParameter);
    if (!params) {
        return 1;
    }

    // Load the model in a thread-safe manner
    gdp1::Model* model =
        new gdp1::Model(params->modelDesc.filepath, params->modelDesc.shader, params->modelDesc.textures);

    // Lock the mutex before modifying shared data
    std::lock_guard<std::mutex> lock(g_Mutex);

    EnterCriticalSection(&g_CriticalSection);

    // Update shared data in a critical section
    params->modelMap.insert(std::make_pair(params->modelDesc.name, model));
    params->vertexCount += model->GetVertexCount();
    params->triangleCount += model->GetTriangleCount();

    LeaveCriticalSection(&g_CriticalSection);

    delete params;

    return 0;
}