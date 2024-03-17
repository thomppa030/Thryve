//
// Created by kprie on 15.03.2024.
//

#ifndef SINGLETIMECOMMANDUTIL_H
#define SINGLETIMECOMMANDUTIL_H
#include "../pch.h"

class SingleTimeCommandUtil {
public:
    static VkCommandBuffer BeginSingleTimeCommands(VkDevice device, VkCommandPool commandPool);
    static void EndSingleTimeCommands(VkDevice device, VkCommandPool commandPool, VkQueue queue, VkCommandBuffer commandBuffer);
};

#endif //SINGLETIMECOMMANDUTIL_H
