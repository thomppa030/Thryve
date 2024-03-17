//
// Created by kprie on 15.03.2024.
//
#include "ThryveApplication.h"

ThryveApplication::ThryveApplication() {
    AppGraphicsContext = std::make_unique<GraphicsContext>(GraphicsContext::Backend::Vulkan);
}

ThryveApplication::~ThryveApplication() {
    AppGraphicsContext.reset();
}

void ThryveApplication::run() const {
    AppGraphicsContext->run();
}
