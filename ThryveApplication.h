//
// Created by kprie on 15.03.2024.
//
#pragma once

#include "GraphicsContext.h"


class ThryveApplication {
public:
    ThryveApplication();
    ~ThryveApplication();

    std::unique_ptr<GraphicsContext> AppGraphicsContext;

    void Run() const;
};