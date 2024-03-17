//
// Created by kprie on 15.03.2024.
//
#pragma once

#include "glm/glm.hpp"

struct UniformBufferObject {
    alignas(16) glm::mat4 model;
    alignas(16) glm::mat4 view;
    alignas(16) glm::mat4 projection;
};

