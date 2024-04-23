//
// Created by kprie on 22.04.2024.
//

#include "Core/Window.h"

#include "Core/GraphicsAPI.h"
#include "Vulkan/VulkanContext.h"

namespace Thryve::Rendering {
    Core::SharedRef<Window> Window::Create(const WindowSettings& settings)
    {
            switch (Core::RenderAPI::CurrentAPI())
            {
            case Core::GraphicsAPIType::NONE:
                return nullptr;
            case Core::GraphicsAPIType::VULKAN:
                return Core::SharedRef<VulkanWindow>::Create(settings);
            }
            return nullptr;
    }
}
