//
// Created by thomppa on 3/23/24.
//

#include "Memory.h"
namespace Thryve::Core::Memory {

    MemoryService::~MemoryService() {}
    void MemoryService::Init(const ServiceConfiguration *configuration) { Service::Init(configuration); }
    void MemoryService::ShutDown() { Service::ShutDown(); }
    Allocator *MemoryService::GetAllocator(AllocatorType type)
    {
        if (const auto _it = allocators.find(type) && _it != allocators.end())
            return _it->second.Get();
        return nullptr;
    }

}

