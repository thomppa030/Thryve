//
// Created by kprie on 16.03.2024.
//
#pragma once

#include "pch.h"
#include "spdlog/spdlog.h"

inline std::string vkResultToString(VkResult result) {
    switch (result) {
        case VK_SUCCESS: return "VK_SUCESS";
        case VK_INCOMPLETE: return "VK_INCOMPLETE";
        case VK_NOT_READY: return "VK_NOT_READY";
        case VK_TIMEOUT: return "VK_TIMEOUT";
        default: return "Result Case not Implemented yet";
    }
}

template<typename Func>
auto VulkanCall(Func func, const char* file, int line, const char* funcsig) ->decltype(func()) {
    auto result = func();
    if (result != VK_SUCCESS) {
        spdlog::error("[{}:{} - {}] Vulkan Call failed with error: {}", file, line, funcsig, vkResultToString(result));
        throw std::runtime_error("Application Terminated");
    }
    return result;
}

#define VK_CALL(func) VulkanCall([&](){return func;}, __FILE__, __LINE__, #func)

