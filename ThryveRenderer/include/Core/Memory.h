//
// Created by thomppa on 3/23/24.
//

#pragma once
#include <map>
#include <string_view>


#include "IService.h"
#include "Ref.h"

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

    class MemoryService final : public IService {
    public:
        ~MemoryService() override;

        void Init(ServiceConfiguration* configuration) override;
        void ShutDown() override;

        template<typename AllocatorT>
        UniqueRef<AllocatorT> GetAllocator() {
            auto type = std::type_index(typeid(AllocatorT));
            auto it = allocators.find(type);
            if (it != allocators.end()) {
                return static_cast<UniqueRef<AllocatorT>>(it->second.Get());
            }
            // Optionally, automatically register a default instance of the allocator
            // if not found. Or return nullptr or throw an exception based on your error handling strategy.
            return nullptr;
        }

        template<typename AllocatorT, typename... Args>
        UniqueRef<AllocatorT> RegisterAllocator(Args&&... args) {
            std::type_index _typeIndex(typeid(AllocatorT));
            auto _it = allocators.find(_typeIndex);
            if (_it == allocators.end()) {
                allocators[_typeIndex] = std::make_unique<AllocatorT>(std::forward<Args>(args)...);
            }
            return static_cast<UniqueRef<AllocatorT>>(allocators[_typeIndex].Get());
        }
    private:
        std::map <std::type_index, UniqueRef<IAllocator>> allocators;
    };
}


