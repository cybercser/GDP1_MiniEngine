#pragma once
// Adapted from Cherno's [OpenGL-Core template](https://github.com/TheCherno/OpenGL)

#include "core.h"
#include "layer.h"

#include <vector>

namespace gdp1 {

class LayerStack {
public:
    LayerStack() = default;
    ~LayerStack();

    void PushLayer(Layer* layer);
    void PushOverlay(Layer* overlay);
    void PopLayer(Layer* layer);
    void PopOverlay(Layer* overlay);

    std::vector<Layer*>::iterator begin() { return m_Layers.begin(); }
    std::vector<Layer*>::iterator end() { return m_Layers.end(); }

    bool IsOverlay(Layer* layer) const;
    bool IsLayer(Layer* layer) const;

private:
    std::vector<Layer*> m_Layers;
    uint32_t m_LayerInsertIndex = 0;
};

}  // namespace gdp1