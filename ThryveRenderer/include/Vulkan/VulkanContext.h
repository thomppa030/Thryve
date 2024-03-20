//
// Created by kprie on 20.03.2024.
//
#pragma once

#include "VulkanDeviceSelector.h"
#include "../../../IRenderContext.h"
#include "Renderer/Renderer.hpp"

namespace Thryve::Rendering {
    class VulkanContext final : public IRenderContext {
    public:
        VulkanContext();
        ~VulkanContext() override;

        void Run() override;

        [[nodiscard]] VulkanDeviceSelector* GetDevice() const {return m_device;}

        static Core::SharedRef<VulkanContext> Get() {
            return static_cast<Core::SharedRef<VulkanContext>>(Renderer::GetContext());
        }

    private:
        VulkanDeviceSelector* m_device;


    };
}
