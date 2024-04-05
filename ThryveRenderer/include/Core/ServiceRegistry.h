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
        template<typename ServiceType, typename... ConstructorArgs>
        static SharedRef<ServiceType> RegisterService(ConstructorArgs&&... args)
        {
            std::type_index typeIndex(typeid(ServiceType));
            auto it = registeredServices.find(typeIndex);
            if (it == registeredServices.end()) {
                SharedRef<ServiceType> instance = SharedRef<ServiceType>::Create(std::forward<ConstructorArgs>(args)...);
                registeredServices[typeIndex] = instance.template StaticCast<ServiceType>(); // Assuming a StaticCast method in SharedRef
                return instance;
            } else {
                // Cast back to the correct type when retrieving an existing service
                return it->second.template DynamicCast<ServiceType>();
            }
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