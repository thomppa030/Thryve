//
// Created by thomppa on 3/23/24.
//

#include "Core/Memory.h"

#include <iostream>

namespace Thryve::Core::Memory {

#pragma region LinearAllocator
    LinearAllocator::LinearAllocator(const size_t size) :
        m_memoryBlock{new std::byte[size]}, m_totalSize{size}, m_allocatedSize{0}
    {
    }

    LinearAllocator::~LinearAllocator() { delete[] m_memoryBlock; }

    void *LinearAllocator::Allocate(const size_t size, const size_t alignment, std::string_view file, int line)
    {
        const auto _currentAddress = reinterpret_cast<size_t>(m_memoryBlock + m_allocatedSize);
        size_t _padding = 0;

        if (alignment != 0)
        {
            size_t _misalignment = (_currentAddress & (alignment - 1));
            _padding = (_misalignment > 0) ? (alignment - _misalignment) : 0;
        }

        if (m_allocatedSize + _padding + size > m_totalSize)
        {
            std::cerr << "LinearAllocator::allocate() - Allocation failed: Not enough memory." << "File: " << file << "Line: " << line << "\n";
            return nullptr;
        }

        m_allocatedSize += _padding;
        void *_allocatedMemory = m_memoryBlock + m_allocatedSize;
        m_allocatedSize += size;

        return _allocatedMemory;
    }

    size_t LinearAllocator::GetTotalAllocated() const { return m_allocatedSize; }

    void LinearAllocator::Reset() { m_allocatedSize = 0; }
#pragma endregion

#pragma region StackAllocator
    StackAllocator::StackAllocator(const size_t size) :
        m_memoryBlock{new std::byte[size]}, m_totalSize{size}, m_allocatedSize{0}
    {
    }

    StackAllocator::~StackAllocator() { delete[] m_memoryBlock; }

    void *StackAllocator::Allocate(const size_t size, const size_t alignment, const std::string_view file,
                                   const int line)
    {
        const auto _currentAddress = reinterpret_cast<size_t>(m_memoryBlock + m_allocatedSize);
        size_t _padding = 0;

        if (alignment != 0)
        {
            size_t _misalignment = (_currentAddress & (alignment - 1));
            _padding = (_misalignment > 0) ? (alignment - _misalignment) : 0;
        }

        if (m_allocatedSize + _padding + size > m_totalSize)
        {
            std::cerr << "StackAllocator::allocate() - Allocation failed: Not enough memory."
                      << "File: " << file << "Line: " << line << "\n";
            return nullptr;
        }

        m_memoryBlock[m_allocatedSize + _padding - 1] = static_cast<std::byte>(_padding);
        void *_allocatedMemory = m_memoryBlock + m_allocatedSize + _padding;

        return _allocatedMemory;
    }

    size_t StackAllocator::GetTotalAllocated() const { return m_allocatedSize; }

    void StackAllocator::Deallocate(void *pointer)
    {
        const auto _currentAddress = reinterpret_cast<size_t>(pointer);
        const auto _blockStartAddress = reinterpret_cast<size_t>(m_memoryBlock);

        // Calculate how far into the Block this Pointer is
        const size_t _offset = _currentAddress - _blockStartAddress;

        // Retrieve the padding stored during allocation
        const auto _padding = static_cast<size_t>(m_memoryBlock[_offset - 1]);

        // Calculate the original allocation Start, including padding
        const size_t _originalAllocationStart = _offset - _padding;

        // This effectively "pops" the last allocation off the stack
        m_allocatedSize = _originalAllocationStart - 1;
    }

    void StackAllocator::Reset() { m_allocatedSize = 0; }
#pragma endregion

#pragma region PoolAllocator

    PoolAllocator::PoolAllocator(size_t objectSize, size_t objectAlignment, size_t size) :
        m_ObjectSize{std::max(objectSize, sizeof(FreeBlock *))}, m_ObjectAlignment{objectAlignment}, m_PoolSize{size},
        m_FreeList{nullptr}
    {
        assert(objectSize >= sizeof(FreeBlock *));
        m_MemoryBlock = operator new(m_ObjectSize * m_PoolSize);
        Reset();
    }

    PoolAllocator::~PoolAllocator() { operator delete(m_MemoryBlock); }

    void *PoolAllocator::Allocate(size_t size, size_t alignment, std::string_view file, int line)
    {
        assert(size == m_ObjectSize && alignment == m_ObjectAlignment);
        if (!m_FreeList)
        {
            std::cerr << "Pool exhausted! " << file << " Line: " << line << "\n";
            return nullptr;
        }
        FreeBlock *_block = m_FreeList;
        m_FreeList = m_FreeList->Next;
        return _block;
    }

    void PoolAllocator::Deallocate(void *pointer)
    {
        const auto _block = static_cast<FreeBlock *>(pointer);
        _block->Next = m_FreeList;
        m_FreeList = _block;
    }

    // This should be Constant, as the pool never grows
    size_t PoolAllocator::GetTotalAllocated() const { return m_PoolSize * m_ObjectSize; }

    void PoolAllocator::Reset()
    {
        // Initialize the free list
        auto _block = static_cast<FreeBlock*>(m_MemoryBlock);
        m_FreeList = _block;

        for (size_t i = 0; i < m_PoolSize - 1; ++i)
        {
            _block->Next = reinterpret_cast<FreeBlock*>(reinterpret_cast<std::byte*>(_block) + m_ObjectSize);
            _block = _block->Next;
        }
        _block->Next = nullptr;
    }


#pragma endregion

#pragma region MemoryService
    MemoryService::~MemoryService() {}
    void MemoryService::Init(const ServiceConfiguration *configuration) { IService::Init(configuration); }
    void MemoryService::ShutDown() { IService::ShutDown(); }
    IAllocator *MemoryService::GetAllocator(AllocatorType type)
    {
        if (const auto _it = allocators.find(type); _it != allocators.end())
            return _it->second.Get();

        return nullptr;
    }
#pragma endregion



}

