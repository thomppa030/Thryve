//
// Created by kprie on 20.03.2024.
//

#include "Vulkan/VulkanContext.h"


namespace Thryve::Rendering {
    VulkanContext::VulkanContext() {
        Init();
    }

    VulkanContext::~VulkanContext() {
        vkDestroyInstance(s_Instance, nullptr);
    }

    void VulkanContext::Init() {
        // m_windowContext = Core::SharedRef<VulkanWindowContext>::Create("Thryve Window", 1920, 1080);
        //
        // m_vulkanInstance = Core::SharedRef<VulkanInstance>::Create();
        // m_vulkanInstance->Init("ThryveStaticRender");
        // s_Instance = m_vulkanInstance->GetInstance();
        //
        // s_Surface = m_windowContext->CreateSurface(s_Instance);
        //
        // m_device = Core::SharedRef<VulkanDeviceSelector>::Create(s_Instance, s_Surface);
        // m_device->PickSuitableDevice(DEVICE_EXTENSIONS, ENABLE_VALIDATION_LAYERS);
    }
}