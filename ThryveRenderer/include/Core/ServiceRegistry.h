//
// Created by kprie on 24.03.2024.
//
#pragma once

#include <map>
#include <typeindex>
#include "Core/IService.h"

#include "Ref.h"

namespace Thryve::Core {

    class ServiceRegistry {
    public:
        template<typename Service>
        static void RegisterService(SharedRef<Service> serviceToRegister)
        {
            const auto _typeIndex = std::type_index(typeid(Service));
            registeredServices[_typeIndex] = serviceToRegister;
        }

        template<typename Service>
        static SharedRef<Service> GetService()
        {
            const auto _typeIndex = std::type_index(typeid(Service));
            const auto _it = registeredServices.find(_typeIndex);
            assert(_it != registeredServices.end() && "Service not registered");
            return SharedRef<Service>(_it->second);
        }

    private:
        static std::map<std::type_index, SharedRef<IService>> registeredServices;
    };
}