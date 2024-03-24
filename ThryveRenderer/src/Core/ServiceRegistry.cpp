//
// Created by thomppa on 3/24/24.
//

#include "Core/ServiceRegistry.h"

namespace Thryve::Core {
    std::map<std::type_index, SharedRef<IService>> ServiceRegistry::registeredServices = {};
}
