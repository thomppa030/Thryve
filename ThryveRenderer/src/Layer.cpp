//
// Created by kprie on 17.04.2024.
//

#include <utility>

#include "../include/Layer.h"

namespace Thryve {
    Layer::Layer(std::string layerName) : m_LayerName{std::move(layerName)} {}
} // namespace Thryve
