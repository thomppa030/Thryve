//
// Created by kprie on 20.03.2024.
//

#include "Renderer/RenderContext.h"

#include "Core/GraphicsAPI.h"
#include "Vulkan/VulkanContext.h"

namespace Thryve::Rendering {
	Core::SharedRef<RenderContext> RenderContext::Create() {
		switch (Core::RenderAPI::CurrentAPI())
		{
		case Core::GraphicsAPIType::NONE:
			return nullptr;
		case Core::GraphicsAPIType::VULKAN:
			return Core::SharedRef<VulkanContext>::Create();
		}
		return nullptr;
	}
}
