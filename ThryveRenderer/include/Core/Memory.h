//
// Created by thomppa on 3/23/24.
//

#pragma once
#include <map>


#include "Ref.h"
#include "Service.h"

namespace Thryve::Core::Memory {

    template<typename T, typename Allocator>
    T* Allocate(Allocator& allocator, const size_t count = 1, size_t alignment = alignof(T))
    {
        void* ptr = allocator.Allocate(sizeof(T) * count, alignment, __FILE__, __LINE__);
        return static_cast<T*>(ptr);
    }

    template<typename T, typename Allocator>
    void Deallocate(Allocator& allocator, T* ptr)
    {
        allocator.Deallocate(static_cast<void*>(ptr));
    }

    enum class AllocatorType {
        LINEAR,
        STACK,
        POOL,
        FREELIST,
        DOUBLESTACK,
        TAGGEDHEAP,
        HEAP
    };

    class IAllocator {
    public:
        virtual ~IAllocator()= default;

        virtual void* Allocate(size_t size, size_t alignment, std::string_view file, int line) = 0;
        [[nodiscard]] virtual size_t GetTotalAllocated() const = 0;
    };

    class IDynamicAllocator : public IAllocator {
    public:
        virtual void Deallocate(void *pointer) = 0;
    };

    /*
     *
     */
    class LinearAllocator final : public IAllocator {
    public:
        explicit LinearAllocator(size_t);
        ~LinearAllocator() override;

        void *Allocate(size_t size, size_t alignment, std::string_view file, int line) override;
        [[nodiscard]] size_t GetTotalAllocated() const override;

        void Reset();

    private:
        std::byte* m_memoryBlock;
        size_t m_totalSize;
        size_t m_allocatedSize;
    };

    class StackAllocator final : public IDynamicAllocator {
    public:
        explicit StackAllocator(size_t size);
        ~StackAllocator() override;

        void *Allocate(size_t size, size_t alignment, std::string_view file, int line) override;

        [[nodiscard]] size_t GetTotalAllocated() const override;

        void Deallocate(void *pointer) override;

        void Reset();

    private:
        std::byte* m_memoryBlock;
        size_t m_totalSize;
        size_t m_allocatedSize;
    };

    class PoolAllocator final : public IDynamicAllocator {
    public:
        PoolAllocator(size_t objectSize, size_t objectAlignment, size_t size);
        ~PoolAllocator() override;

        void *Allocate(size_t size, size_t alignment, std::string_view file, int line) override;
        void Deallocate(void *pointer) override;

        [[nodiscard]] size_t GetTotalAllocated() const override;

        void Reset();

    private:
        struct FreeBlock {
            FreeBlock *Next;
        };

        size_t m_ObjectSize;
        size_t m_ObjectAlignment;
        size_t m_PoolSize;
        void *m_MemoryBlock;

        FreeBlock* m_FreeList;
    };



    struct MemoryServiceConfiguration final : ServiceConfiguration {
        size_t MaximumDynamicSize = 32*1024*1024;
    };

    class MemoryService final : public Service {
    public:
        ~MemoryService() override;

        void Init(const ServiceConfiguration* configuration) override;
        void ShutDown() override;

        IAllocator* GetAllocator(AllocatorType type);

        template<typename AllocatorT, typename ...Args>
        void RegisterAllocator(const AllocatorType type, Args&... args)
        {
            allocators[type] = UniqueRef<AllocatorT>::Create(args);
        }

    private:
        std::map <AllocatorType, UniqueRef<IAllocator>> allocators;
    };
}


