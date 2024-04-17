//
// Created by kprie on 17.04.2024.
//

#include "LayerStack.h"
#include "Layer.h"

namespace Thryve {
    LayerStack::~LayerStack()
    {
        for (auto* _layer : m_Layers)
        {
            _layer->OnDetach();
            delete _layer;
        }
    }

    void LayerStack::PushLayer(Layer* layer)
    {
        m_Layers.emplace(m_Layers.begin() + m_LayerInsertIndex, layer);
        layer->OnAttach();
        m_LayerInsertIndex++;
    }

    void LayerStack::PushOverlay(Layer* overlay)
    {
        m_Layers.emplace_back(overlay);
    }

    void LayerStack::PopLayer(Layer* layer)
    {
        const auto _it = std::ranges::find(m_Layers, layer);
        if (_it != m_Layers.end())
        {
            m_Layers.erase(_it);
            --m_LayerInsertIndex;
            layer->OnDetach();
        }
    }
    void LayerStack::PopOverlay(Layer* overlay)
    {
        const auto _it = std::ranges::find(m_Layers, overlay);
        if (_it != m_Layers.end())
        {
            m_Layers.erase(_it);
            overlay->OnDetach();
        }
    }
} // Thryve