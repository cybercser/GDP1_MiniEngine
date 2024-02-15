// Adapted from Cherno's [OpenGL-Core template](https://github.com/TheCherno/OpenGL)
#include "layer_stack.h"

namespace gdp1 {

LayerStack::~LayerStack() {
    for (Layer* layer : m_Layers) {
        delete layer;
    }
}

// layers get pushed to the first half of the vector
void LayerStack::PushLayer(Layer* layer) {
    m_Layers.emplace(m_Layers.begin() + m_LayerInsertIndex, layer);
    m_LayerInsertIndex++;
    layer->OnAttach();
}

// overlays get pushed to the second half of the vector
// overlays are always on top of layers
void LayerStack::PushOverlay(Layer* overlay) {
    m_Layers.emplace_back(overlay);
    overlay->OnAttach();
}

void LayerStack::PopLayer(Layer* layer) {
    std::vector<Layer*>::iterator it = std::find(m_Layers.begin(), m_Layers.begin() + m_LayerInsertIndex, layer);
    if (it != m_Layers.begin() + m_LayerInsertIndex) {
        layer->OnDetach();
        m_Layers.erase(it);
        m_LayerInsertIndex--;
    }
}

void LayerStack::PopOverlay(Layer* overlay) {
    std::vector<Layer*>::iterator it = std::find(m_Layers.begin() + m_LayerInsertIndex, m_Layers.end(), overlay);
    if (it != m_Layers.end()) {
        overlay->OnDetach();
        m_Layers.erase(it);
    }
}

bool LayerStack::IsOverlay(Layer* layer) const {
    // if the layer is in the second half of the vector, it is an overlay
    return std::find(m_Layers.begin() + m_LayerInsertIndex, m_Layers.end(), layer) != m_Layers.end();
}

bool LayerStack::IsLayer(Layer* layer) const {
    // if the layer is in the first half of the vector, it is a layer
    return std::find(m_Layers.begin(), m_Layers.begin() + m_LayerInsertIndex, layer) !=
           m_Layers.begin() + m_LayerInsertIndex;
}

}  // namespace gdp1
