//
// Created by kprie on 14.03.2024.
//

#include "Vulkan/VulkanDeviceSelector.h"

#include <set>
#include <stdexcept>

#include "Core/App.h"
#include "utils/VkDebugUtils.h"

VulkanDeviceSelector::VulkanDeviceSelector(const VkInstance instance
                                           , const VkSurfaceKHR surface) : m_instance(instance),
                                                                           m_surface(surface), m_graphicsQueue(nullptr),
                                                                           m_presentQueue(nullptr) {
}

VulkanDeviceSelector::~VulkanDeviceSelector() {
    if (m_logicalDevice != VK_NULL_HANDLE) {
        vkDestroyDevice(m_logicalDevice, nullptr); // Destroy the logical device
        m_logicalDevice = VK_NULL_HANDLE;
    }
}

VulkanDeviceSelector::VulkanDeviceSelector(VulkanDeviceSelector && other) noexcept
    : m_instance(other.m_instance),
  m_surface(other.m_surface),
  m_physicalDevice(other.m_physicalDevice),
  m_logicalDevice(other.m_logicalDevice),
  m_graphicsQueue(other.m_graphicsQueue),
  m_presentQueue(other.m_presentQueue) {

        // Invalidate the moved-from object's Vulkan handles to ensure it doesn't destroy them.
        other.m_instance = VK_NULL_HANDLE;
        other.m_surface = VK_NULL_HANDLE;
        other.m_physicalDevice = VK_NULL_HANDLE;
        other.m_logicalDevice = VK_NULL_HANDLE;
        other.m_graphicsQueue = VK_NULL_HANDLE;
        other.m_presentQueue = VK_NULL_HANDLE;
}

VulkanDeviceSelector & VulkanDeviceSelector::operator=(VulkanDeviceSelector && other) noexcept {
    if (this != &other) { // Prevent self-assignment
        // Clean up the current resources
        if (m_logicalDevice != VK_NULL_HANDLE) {
            vkDestroyDevice(m_logicalDevice, nullptr); // Only destroy the logical device; Vulkan cleans up the physical device
            m_logicalDevice = VK_NULL_HANDLE; // Ensure we mark it as no longer valid
        }
        // Note: Depending on the design, you might also need to handle cleanup for other resources that this class owns.

        // Transfer ownership of resources from other to this
        m_instance = other.m_instance;
        m_surface = other.m_surface;
        m_physicalDevice = other.m_physicalDevice;
        m_logicalDevice = other.m_logicalDevice;
        m_graphicsQueue = other.m_graphicsQueue;
        m_presentQueue = other.m_presentQueue;

        // Invalidate the moved-from object to prevent it from freeing resources that are now owned by this
        other.m_instance = VK_NULL_HANDLE;
        other.m_surface = VK_NULL_HANDLE;
        other.m_physicalDevice = VK_NULL_HANDLE;
        other.m_logicalDevice = VK_NULL_HANDLE;
        other.m_graphicsQueue = VK_NULL_HANDLE;
        other.m_presentQueue = VK_NULL_HANDLE;
    }
    return *this;
}


VkDevice VulkanDeviceSelector::GetLogicalDevice() {
    if (m_logicalDevice)
    {
        return m_logicalDevice;
    }
    throw std::runtime_error("No Logical Device!");
}

VkPhysicalDevice VulkanDeviceSelector::GetPhysicalDevice() const {
    return m_physicalDevice;
}

VkQueue VulkanDeviceSelector::GetGraphicsQueue() const {
    return m_graphicsQueue;
}

VkQueue VulkanDeviceSelector::GetPresentQueue() const { return m_presentQueue; }

std::string VulkanDeviceSelector::GetGraphicsCardType(VkPhysicalDeviceProperties props)
{
    switch (props.deviceType)
    {
    case VK_PHYSICAL_DEVICE_TYPE_INTEGRATED_GPU:
        return"Integrated";
    case VK_PHYSICAL_DEVICE_TYPE_DISCRETE_GPU:
        return"Discrete";
    case VK_PHYSICAL_DEVICE_TYPE_VIRTUAL_GPU:
        return"Virtual";
    case VK_PHYSICAL_DEVICE_TYPE_CPU:
        return  "CPU";
    default:
        return"Other";
    }
}
void VulkanDeviceSelector::PickSuitableDevice(const std::vector<const char *> &deviceExtensions
                                              , bool enableValidationLayers) {
        uint32_t deviceCount = 0;
        VK_CALL(vkEnumeratePhysicalDevices(m_instance, &deviceCount, nullptr));

        if (deviceCount == 0) {
            throw std::runtime_error("failed to find GPUs with Vulkan support!");
        }

        std::vector<VkPhysicalDevice> devices(deviceCount);
        VK_CALL(vkEnumeratePhysicalDevices(m_instance, &deviceCount, devices.data()));

        for (const auto& device : devices) {
            if (IsDeviceSuitable(device, deviceExtensions)) {
                m_physicalDevice = device;
                break;
            }
        }

        if (m_physicalDevice == VK_NULL_HANDLE) {
            throw std::runtime_error("failed to find a suitable GPU!");
        }

        VkPhysicalDeviceProperties _props;
        vkGetPhysicalDeviceProperties(m_physicalDevice, &_props);
        Thryve::Core::App::SetGraphicsCardName(_props.deviceName);

        const std::string _graphicsCardTypeAsString = GetGraphicsCardType(_props);
        Thryve::Core::App::SetGraphicsCardType(_graphicsCardTypeAsString);

        CreateLogicalDevice(m_physicalDevice, deviceExtensions, enableValidationLayers);
}

SwapChainSupportDetails VulkanDeviceSelector::QuerySwapChainSupport(const VkPhysicalDevice device) const {
    SwapChainSupportDetails _details;

    VK_CALL(vkGetPhysicalDeviceSurfaceCapabilitiesKHR(device, m_surface, &_details.Capabilities));

    uint32_t formatCount;
    VK_CALL(vkGetPhysicalDeviceSurfaceFormatsKHR(device, m_surface, &formatCount, nullptr));

    if (formatCount != 0) {
        _details.Formats.resize(formatCount);
        VK_CALL(vkGetPhysicalDeviceSurfaceFormatsKHR(device, m_surface, &formatCount, _details.Formats.data()));
    }

    uint32_t presentModeCount;
    VK_CALL(vkGetPhysicalDeviceSurfacePresentModesKHR(device, m_surface, &presentModeCount, nullptr));

    if (presentModeCount != 0) {
        _details.PresentModes.resize(presentModeCount);
        VK_CALL(vkGetPhysicalDeviceSurfacePresentModesKHR(device, m_surface, &presentModeCount, _details.PresentModes.data()));
    }
    return _details;
}

bool VulkanDeviceSelector::
IsDeviceSuitable(VkPhysicalDevice device, const std::vector<const char *> &deviceExtensions) {
        m_queueFamiliyIndices = FindQueueFamilies(device);

        bool extensionsSupported = CheckDeviceExtensionSupport(device, deviceExtensions);

        bool swapChainAdequate = false;

        if (extensionsSupported) {
            SwapChainSupportDetails swapChainSupport = QuerySwapChainSupport(device);
            swapChainAdequate = !swapChainSupport.Formats.empty() && !swapChainSupport.PresentModes.empty();
        }

        VkPhysicalDeviceFeatures supportedFeatures;
        vkGetPhysicalDeviceFeatures(device, &supportedFeatures);

        return m_queueFamiliyIndices.IsComplete() && extensionsSupported && swapChainAdequate && supportedFeatures.samplerAnisotropy;
}

bool VulkanDeviceSelector::CheckDeviceExtensionSupport(VkPhysicalDevice device
    , const std::vector<const char *> &deviceExtensions) {
        uint32_t extensionCount;
        VK_CALL(vkEnumerateDeviceExtensionProperties(device, nullptr, &extensionCount, nullptr));

        std::vector<VkExtensionProperties> availableExtensions(extensionCount);
        VK_CALL(vkEnumerateDeviceExtensionProperties(device, nullptr, &extensionCount, availableExtensions.data()));

        std::set<std::string> requiredExtensions(deviceExtensions.begin(), deviceExtensions.end());

        for (const auto& extension : availableExtensions) {
            requiredExtensions.erase(extension.extensionName);
        }

        return requiredExtensions.empty();
}

QueueFamilyIndices VulkanDeviceSelector::FindQueueFamilies(VkPhysicalDevice device) const {
        QueueFamilyIndices indices;

        uint32_t queueFamilyCount = 0;
        vkGetPhysicalDeviceQueueFamilyProperties(device, &queueFamilyCount, nullptr);

        std::vector<VkQueueFamilyProperties> queueFamilies(queueFamilyCount);
        vkGetPhysicalDeviceQueueFamilyProperties(device, &queueFamilyCount, queueFamilies.data());

        int i = 0;
        for (const auto& queueFamily : queueFamilies) {
            if (queueFamily.queueFlags & VK_QUEUE_GRAPHICS_BIT) {
                indices.GraphicsFamily = i;
            }

            VkBool32 presentSupport = false;
            VK_CALL(vkGetPhysicalDeviceSurfaceSupportKHR(device, i, m_surface, &presentSupport));

            if (presentSupport) {
                indices.PresentFamily = i;
            }

            if (indices.IsComplete()) {
                break;
            }

            i++;
        }

        return indices;
}

void VulkanDeviceSelector::CreateLogicalDevice(VkPhysicalDevice physicalDevice
    , const std::vector<const char *> &deviceExtensions, bool enableValidationLayers) {

        QueueFamilyIndices indices = FindQueueFamilies(physicalDevice);

        std::vector<VkDeviceQueueCreateInfo> queueCreateInfos;
        std::set<uint32_t> uniqueQueueFamilies = {indices.GraphicsFamily.value(), indices.PresentFamily.value()};

        float queuePriority = 1.0f;
        for (uint32_t queueFamily : uniqueQueueFamilies) {
            VkDeviceQueueCreateInfo queueCreateInfo{};
            queueCreateInfo.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
            queueCreateInfo.queueFamilyIndex = queueFamily;
            queueCreateInfo.queueCount = 1;
            queueCreateInfo.pQueuePriorities = &queuePriority;
            queueCreateInfos.push_back(queueCreateInfo);
        }

        VkPhysicalDeviceFeatures deviceFeatures{};
        deviceFeatures.samplerAnisotropy = VK_TRUE;

        VkDeviceCreateInfo createInfo{};
        createInfo.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;

        createInfo.queueCreateInfoCount = static_cast<uint32_t>(queueCreateInfos.size());
        createInfo.pQueueCreateInfos = queueCreateInfos.data();

        createInfo.pEnabledFeatures = &deviceFeatures;

        createInfo.enabledExtensionCount = static_cast<uint32_t>(deviceExtensions.size());
        createInfo.ppEnabledExtensionNames = deviceExtensions.data();

        if (enableValidationLayers) {
            createInfo.enabledLayerCount = static_cast<uint32_t>(VALIDATION_LAYERS.size());
            createInfo.ppEnabledLayerNames = VALIDATION_LAYERS.data();
        } else {
            createInfo.enabledLayerCount = 0;
        }


        VK_CALL(vkCreateDevice(physicalDevice, &createInfo, nullptr, &m_logicalDevice));

        vkGetDeviceQueue(m_logicalDevice, indices.GraphicsFamily.value(), 0, &m_graphicsQueue);
        vkGetDeviceQueue(m_logicalDevice, indices.PresentFamily.value(), 0, &m_presentQueue);
}
