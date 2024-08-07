//
// Created by kprie on 14.03.2024.
//

#include "Vulkan/VulkanInstance.h"

#include <iostream>
#include <stdexcept>
#include <vulkan/vulkan.hpp>

#include "Core/Log.h"
#include "Core/ServiceRegistry.h"
#include "GLFW/glfw3.h"
#include "utils/VkDebugUtils.h"

namespace Thryve::Rendering {
    VkResult CreateDebugUtilsMessengerEXT(const VkInstance instance,
                                          const VkDebugUtilsMessengerCreateInfoEXT *pCreateInfo,
                                          const VkAllocationCallbacks *pAllocator,
                                          VkDebugUtilsMessengerEXT *pDebugMessenger)
    {
        if (const auto func = reinterpret_cast<PFN_vkCreateDebugUtilsMessengerEXT>(
                vkGetInstanceProcAddr(instance, "vkCreateDebugUtilsMessengerEXT"));
            func != nullptr)
        {
            return func(instance, pCreateInfo, pAllocator, pDebugMessenger);
        }
        else
        {
            return VK_ERROR_EXTENSION_NOT_PRESENT;
        }
    }

    void DestroyDebugUtilsMessengerEXT(const VkInstance instance, const VkDebugUtilsMessengerEXT debugMessenger,
                                       const VkAllocationCallbacks *pAllocator)
    {
        if (const auto func = reinterpret_cast<PFN_vkDestroyDebugUtilsMessengerEXT>(
                vkGetInstanceProcAddr(instance, "vkDestroyDebugUtilsMessengerEXT"));
            func != nullptr)
        {
            func(instance, debugMessenger, pAllocator);
        }
    }

    VulkanInstance::VulkanInstance() : m_enableValidationLayers(true), debugMessenger(nullptr)
    {
        m_validationLayers = {"VK_LAYER_KHRONOS_validation"};
    }

    VulkanInstance::~VulkanInstance()
    {
        if (m_enableValidationLayers)
        {
            DestroyDebugUtilsMessengerEXT(m_instance, debugMessenger, nullptr);
        }

        if (m_instance != VK_NULL_HANDLE)
        {
            vkDestroyInstance(m_instance, nullptr);
        }
    }

    void VulkanInstance::Init(const std::string &applicationName)
    {
        if (m_enableValidationLayers && !CheckValidationLayerSupport())
        {
            throw std::runtime_error("Validation layers requested, but not available!");
        }
        CreateInstance(applicationName);
        if (m_enableValidationLayers)
        {
            SetupDebugMessenger();
        }
    }

    void VulkanInstance::PopulateDebugMessengerCreateInfo(VkDebugUtilsMessengerCreateInfoEXT &createInfo)
    {
        createInfo = {};
        createInfo.sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_MESSENGER_CREATE_INFO_EXT;
        createInfo.messageSeverity = VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT |
            VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT;
        createInfo.messageType = VK_DEBUG_UTILS_MESSAGE_TYPE_GENERAL_BIT_EXT |
            VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT;
        createInfo.pfnUserCallback = DebugCallback;
    }

    void VulkanInstance::CreateInstance(const std::string &applicationName)
    {
        if (m_enableValidationLayers && !CheckValidationLayerSupport())
        {
            throw std::runtime_error("validation layers requested, but not available!");
        }

        VkApplicationInfo appInfo{};
        appInfo.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
        appInfo.pApplicationName = applicationName.c_str();
        appInfo.applicationVersion = VK_MAKE_VERSION(1, 0, 0);
        appInfo.pEngineName = "No Engine";
        appInfo.engineVersion = VK_MAKE_VERSION(1, 0, 0);
        appInfo.apiVersion = VK_API_VERSION_1_0;

        VkInstanceCreateInfo createInfo{};
        createInfo.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
        createInfo.pApplicationInfo = &appInfo;

        const auto extensions = GetRequiredExtensions();
        createInfo.enabledExtensionCount = static_cast<uint32_t>(extensions.size());
        createInfo.ppEnabledExtensionNames = extensions.data();

        VkDebugUtilsMessengerCreateInfoEXT debugCreateInfo{};
        if (m_enableValidationLayers)
        {
            createInfo.enabledLayerCount = static_cast<uint32_t>(m_validationLayers.size());
            createInfo.ppEnabledLayerNames = m_validationLayers.data();

            PopulateDebugMessengerCreateInfo(debugCreateInfo);
            createInfo.pNext = (VkDebugUtilsMessengerCreateInfoEXT *)&debugCreateInfo;
        }
        else
        {
            createInfo.enabledLayerCount = 0;

            createInfo.pNext = nullptr;
        }

        VK_CALL(vkCreateInstance(&createInfo, nullptr, &m_instance));
    }

    bool VulkanInstance::CheckValidationLayerSupport() const
    {
        uint32_t layerCount;
        VK_CALL(vkEnumerateInstanceLayerProperties(&layerCount, nullptr));

        std::vector<VkLayerProperties> availableLayers(layerCount);
        VK_CALL(vkEnumerateInstanceLayerProperties(&layerCount, availableLayers.data()));

        for (const char *layerName : m_validationLayers)
        {
            bool layerFound = false;

            for (const auto &layerProperties : availableLayers)
            {
                if (strcmp(layerName, layerProperties.layerName) == 0)
                {
                    layerFound = true;
                    break;
                }
            }

            if (!layerFound)
            {
                return false;
            }
        }

        return true;
    }

    std::vector<const char *> VulkanInstance::GetRequiredExtensions() const
    {
        uint32_t glfwExtensionCount = 0;
        const char **glfwExtensions = glfwGetRequiredInstanceExtensions(&glfwExtensionCount);

        std::vector<const char *> extensions(glfwExtensions, glfwExtensions + glfwExtensionCount);

        if (m_enableValidationLayers)
        {
            extensions.push_back(VK_EXT_DEBUG_UTILS_EXTENSION_NAME);
        }

        return extensions;
    }

    void VulkanInstance::SetupDebugMessenger()
    {
        if (!m_enableValidationLayers)
            return;

        VkDebugUtilsMessengerCreateInfoEXT createInfo;
        PopulateDebugMessengerCreateInfo(createInfo);

        VK_CALL(CreateDebugUtilsMessengerEXT(m_instance, &createInfo, nullptr, &debugMessenger));
    }

    enum class LogMessageSeverity {
        DEBUG,
        INFO,
        WARN,
        ERROR,
        UNKNOWN
    };

    LogMessageSeverity GetMessageSeverityLevel(VkDebugUtilsMessageSeverityFlagBitsEXT messageSeverity)
    {
        if (messageSeverity & VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT)
        {
            return LogMessageSeverity::DEBUG;
        }
        else if (messageSeverity & VK_DEBUG_UTILS_MESSAGE_SEVERITY_INFO_BIT_EXT)
        {
            return LogMessageSeverity::INFO;
        }
        else if (messageSeverity & VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT)
        {
            return LogMessageSeverity::WARN;
        }
        else if (messageSeverity & VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT)
        {
            return LogMessageSeverity::ERROR;
        }
        return LogMessageSeverity::UNKNOWN;
    }

    VkBool32 VulkanInstance::DebugCallback(VkDebugUtilsMessageSeverityFlagBitsEXT messageSeverity,
                                           VkDebugUtilsMessageTypeFlagsEXT messageType,
                                           const VkDebugUtilsMessengerCallbackDataEXT *pCallbackData, void *pUserData)
    {
        const auto _eMessageSeverity = GetMessageSeverityLevel(messageSeverity);

        if (auto _validationLogger = Core::ServiceRegistry::GetService<Core::ValidationLayerLogger>())
        {
            switch (_eMessageSeverity)
            {
            case LogMessageSeverity::DEBUG:
                _validationLogger->LogDebug(pCallbackData->pMessage);
                break;
            case LogMessageSeverity::INFO:
                _validationLogger->LogInfo(pCallbackData->pMessage);
                break;
            case LogMessageSeverity::WARN:
                _validationLogger->LogWarning(pCallbackData->pMessage);
                break;
            case LogMessageSeverity::ERROR:
                _validationLogger->LogError(pCallbackData->pMessage);
                break;
            case LogMessageSeverity::UNKNOWN:
                _validationLogger->LogFatal(pCallbackData->pMessage);
                break;
            default:;
                _validationLogger->LogDebug(pCallbackData->pMessage);
            }
        }
        else
        {
            std::cerr
                << "Failed to find Logger for Validation Layer, ensure all Logging Services are set up correctly!";
        }

        return VK_FALSE;
    }

} // namespace Thryve::Rendering
