//
// Created by thomppa on 3/23/24.
//

#pragma once
#include <map>


#include "Ref.h"
#include "Service.h"

namespace Thryve::Core::Memory {

    enum class AllocatorType {
        LINEAR,
        STACK,
        HEAP
    };

    class Allocator {
    public:
        virtual ~Allocator()= default;

        virtual void* Allocate(size_t size, size_t alignment, std::string_view file, int line) = 0;
        virtual void Deallocate(void* pointer) = 0;
        [[nodiscard]] virtual size_t GetTotalAllocated() const = 0;

    };

    class LinearAllocator final : public Allocator {
    public:
        explicit LinearAllocator(size_t);
        ~LinearAllocator() override;

        void *Allocate(size_t size, size_t alignment, std::string_view file, int line) override;
        void Deallocate(void *pointer) override;
        [[nodiscard]] size_t GetTotalAllocated() const override;

        void Reset();

    private:
        UniqueRef<char[]> memoryBlock;
        size_t totalSize;
        size_t allocatedSize;
    };

    struct MemoryServiceConfiguration final : ServiceConfiguration {
        size_t MaximumDynamicSize = 32*1024*1024;
    };

    class MemoryService final : public Service {
    public:
        ~MemoryService() override;

        void Init(const ServiceConfiguration* configuration) override;
        void ShutDown() override;

        Allocator* GetAllocator(AllocatorType type);

        template<typename AllocatorT, typename ...Args>
        void RegisterAllocator(const AllocatorType type, Args&... args)
        {
            allocators[type] = UniqueRef<AllocatorT>::Create(args);
        }

    private:
        std::map <AllocatorType, UniqueRef<Allocator>> allocators;
    };
}


