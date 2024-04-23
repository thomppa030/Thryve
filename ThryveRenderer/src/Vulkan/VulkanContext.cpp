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
        m_device.Reset();
        m_vulkanInstance.Reset();
    }

    void VulkanContext::Init()
    {
        const Core::SharedRef<Window> _appWindow = Core::App::Get().GetWindow();
        Core::SharedRef<VulkanWindow> _window = _appWindow.As<VulkanWindow>();

        s_Window = static_cast<GLFWwindow*>(_window->GetWindow());
        //
        m_vulkanInstance = Core::SharedRef<VulkanInstance>::Create();
        m_vulkanInstance->Init("ThryveStaticRender");
        s_Instance = m_vulkanInstance->GetInstance();

        s_Surface = _window->CreateSurface();

        m_device = Core::SharedRef<VulkanDeviceSelector>::Create(s_Instance, s_Surface);
        m_device->PickSuitableDevice(DEVICE_EXTENSIONS, ENABLE_VALIDATION_LAYERS);
    }

    void VulkanContext::Run()
    {
        m_renderContext->Run();
    }
} // namespace Thryve::Rendering
