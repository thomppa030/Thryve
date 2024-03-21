//
// Created by kprie on 20.03.2024.
//

#include "Vulkan/VulkanContext.h"


namespace Thryve::Rendering {
    VulkanContext::VulkanContext() {
        m_renderContext = Core::SharedRef<VulkanRenderContext>::Create();
        Init();
    }

    VulkanContext::~VulkanContext() {
        vkDestroySurfaceKHR(s_Instance,s_Surface, nullptr);
        m_windowContext.Reset();
        m_device.Reset();
        m_vulkanInstance.Reset();
    }

    void VulkanContext::Init()
    {
        m_windowContext = Core::SharedRef<VulkanWindowContext>::Create("Thryve Window", 1920, 1080);
        //
        m_vulkanInstance = Core::SharedRef<VulkanInstance>::Create();
        m_vulkanInstance->Init("ThryveStaticRender");
        s_Instance = m_vulkanInstance->GetInstance();

        s_Surface = m_windowContext->CreateSurface();

        m_device = Core::SharedRef<VulkanDeviceSelector>::Create(s_Instance, s_Surface);
        m_device->PickSuitableDevice(DEVICE_EXTENSIONS, ENABLE_VALIDATION_LAYERS);

        
    }
    void VulkanContext::Run()
    {
        m_renderContext->Run();
    }
} // namespace Thryve::Rendering
