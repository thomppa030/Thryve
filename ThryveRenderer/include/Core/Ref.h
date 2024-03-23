//
// Created by kprie on 20.03.2024.
//
#pragma once

#include <atomic>
#include <mutex>
#include <unordered_set>
#include <utility>

#include "glm/gtc/constants.hpp"

namespace Thryve {
    static std::unordered_set<void*> s_LiveReferences;
    static std::mutex s_LiveReferenceMutex;
}

namespace Thryve::Utils::References {
    void AddToLiveReferences(void* instance);
    void RemoveFromeLiveReferences(void* instance);
    bool IsLive(void* instance);

    inline void AddToLiveReferences(void* instance) {
        std::scoped_lock<std::mutex> _lock(s_LiveReferenceMutex);
        s_LiveReferences.insert(instance);
    }

    inline void RemoveFromeLiveReferences(void* instance) {
        std::scoped_lock<std::mutex> _lock(s_LiveReferenceMutex);
        s_LiveReferences.erase(instance);
    }

    inline bool IsLive(void* instance) {
        return s_LiveReferences.contains(instance);
    }
}

namespace Thryve::Core {
    class ReferenceCounted {
    public:
        virtual ~ReferenceCounted() = default;

        void IncrementReferenceCount() const {
            m_ReferenceCount.fetch_add(1, std::memory_order_relaxed);
        }
        void DecrementReferenceCount() const {
            m_ReferenceCount.fetch_sub(1, std::memory_order_acq_rel);
        }

        uint32_t GetReferenceCount() const {return m_ReferenceCount.load(std::memory_order_relaxed);}

        void IncrementWeakCount() const
        {
            m_WeakReferenceCount.fetch_add(1, std::memory_order::relaxed);
        }

        void DecrementWeakCount() const
        {
            m_WeakReferenceCount.fetch_sub(1, std::memory_order::acq_rel);
        }

        uint32_t GetWeakCount() const {return m_WeakReferenceCount.load(std::memory_order::relaxed);}

    private:
        mutable std::atomic<uint32_t> m_ReferenceCount{0};
        mutable std::atomic<uint32_t> m_WeakReferenceCount{0};
    };

    template<typename BaseType>
    class SharedRef {
    public:
        /**
         * @brief Default constructor for SharedRef
         *
         * This constructor initializes the m_Instance member of the SharedRef object to nullptr.
         */
        SharedRef() : m_Instance{nullptr} {
        };

        /**
         * @brief Constructs a SharedRef object from a nullptr.
         *
         * The nullptr parameter is used to initialize the m_Instance member of the SharedRef object.
         *
         * @param nullptr_ The nullptr parameter.
         */
        SharedRef(std::nullptr_t nullptr_) : m_Instance{nullptr} {
        };

        /**
         * @brief Constructs a SharedRef object from a raw pointer of a BaseType object.
         *
         * The given raw pointer is used to initialize the m_Instance member of the SharedRef object.
         * It also checks if the BaseType class implements ReferenceCounting.
         *
         * @param rawPointer The raw pointer of a BaseType object.
         *
         * @note The BaseType class must be derived from the ReferenceCounted abstract class, otherwise a compilation error will occur.
         *       Be sure to derive the BaseType class from ReferenceCounted abstract class to enable reference counting.
         */
        SharedRef(BaseType* rawPointer) : m_Instance{rawPointer} {
            static_assert(std::is_base_of_v<ReferenceCounted, BaseType>
                          , "Baseclass does not implement ReferenceCounting, be sure to derive it from ReferenceCounting abstract class!")
                ;
            IncrementReferenceCount();
        };
        /**
         * @brief Constructs a SharedRef object from an existing SharedRef object of a different derived type.
         *
         * @tparam DerivedType The derived type of the input SharedRef object.
         * @param other The input SharedRef object of a different derived type.
         */
        template <typename DerivedType>
        SharedRef(const SharedRef<DerivedType>& other) {
                m_Instance = dynamic_cast<BaseType*>(other.m_Instance);
                IncrementReferenceCount();
        }

        template <typename DerivedType>
        SharedRef(const SharedRef<DerivedType>&& other) {
            m_Instance = (BaseType*)other.m_Instance;
            other.m_Instance = nullptr;
        }

        ~SharedRef() {
            DecrementReferenceCount();
        }

        SharedRef(const SharedRef<BaseType>& other) : m_Instance{other.m_Instance} {
            IncrementReferenceCount();
        }

        /**
         * @brief Assignment operator for SharedRef.
         *
         * This operator assigns a nullptr value to the SharedRef object.
         * It first decrements the reference count of the m_Instance member,
         * then sets it to nullptr, and finally returns a reference to the updated SharedRef object.
         *
         * @param nullptr_ The nullptr value to assign.
         * @return A reference to the updated SharedRef object.
         */
        SharedRef& operator=(std::nullptr_t) {
            DecrementReferenceCount();
            m_Instance = nullptr;
            return *this;
        }

        SharedRef& operator=(const SharedRef<BaseType>& other)
        {
            if (this == &other)
                return *this;

            other.IncrementReferenceCount();
            DecrementReferenceCount();

            m_Instance = other.m_Instance;
            return *this;
        }

        /**
         * @brief Assignment operator for SharedRef.
         *
         * This operator assigns the m_Instance member of the given SharedRef object to the m_Instance member of this SharedRef object.
         * It first checks if the given SharedRef object is not the same as this SharedRef object.
         * If they are not the same, it increments the reference count of the given SharedRef object,
         * decrements the reference count of this SharedRef object,
         * and then assigns the m_Instance member of the given SharedRef object to the m_Instance member of this SharedRef object.
         * Finally, it returns a reference to the updated SharedRef object.
         *
         * @tparam DerivedType The derived type of the given SharedRef object.
         * @param other The SharedRef object to be assigned.
         * @return A reference to the updated SharedRef object.
         */
        template<typename DerivedType>
        SharedRef& operator=(const SharedRef<DerivedType>& other) {
            other.IncrementReferenceCount();
            DecrementReferenceCount();
            m_Instance = other.m_Instance;
            return *this;
        }

        template<typename DerivedType>
        SharedRef& operator=(const SharedRef<DerivedType>&& other) {
            DecrementReferenceCount();
            m_Instance = (BaseType*)other.m_Instance;
            other.m_Instance =  nullptr;
            return *this;
        }

        operator bool() {return m_Instance != nullptr;}
        operator bool() const {return m_Instance != nullptr;}

        BaseType* operator->() { return m_Instance; }
        const BaseType* operator->() const { return m_Instance; }

        BaseType& operator*() { return *m_Instance; }
        const BaseType& operator*() const { return *m_Instance; }

        BaseType* Raw() {return m_Instance;}
        const BaseType* Raw() const {return m_Instance;}

        void Reset(BaseType* instance = nullptr) {
            DecrementReferenceCount();
            m_Instance = instance;
        }

        template<typename DerivedType>
        SharedRef<DerivedType> As() const {
            return SharedRef<DerivedType>(*this);
        }

        bool IsEqual(const SharedRef<BaseType>& other) {
            if (!m_Instance || !other.m_Instance)
            {
                return false;
            }
            return *m_Instance == *other.m_Instance;
        }

        template<typename... Args>
        static SharedRef<BaseType> Create(Args&&... args) {
            return SharedRef<BaseType>(new BaseType(std::forward<Args>(args)...));
        }

        bool operator==(const SharedRef& other) const {
            return m_Instance == other.m_Instance;
        }

        bool operator!=(const SharedRef& other) const {
            return !(*this == other);
        }

    private:
        void IncrementReferenceCount() const {
            if (m_Instance)
            {
                m_Instance->IncrementReferenceCount();
                Utils::References::AddToLiveReferences(m_Instance);
            }
        }
        void DecrementReferenceCount() const {
            if (m_Instance)
            {
                m_Instance->DecrementReferenceCount();
                if (m_Instance->GetReferenceCount() == 0)
                {
                    delete m_Instance;
                    Utils::References::RemoveFromeLiveReferences(static_cast<void*>(m_Instance));
                    m_Instance = nullptr;
                }
            }
        }
        void IncrementWeakCount() const
        {
            if (m_Instance)
            {
                m_Instance->IncrementWeakCount();
            }
        }

        void DecrementWeakCount() const
        {
            if (m_Instance)
            {
                m_Instance->DecrementWeakCount();
            }
        }

        template<class DerivedType>
        friend class SharedRef;

        mutable BaseType* m_Instance;
    };
    //TODO UniqueRef
    template<typename BaseType>
    class UniqueRef {
    public:
        explicit UniqueRef(BaseType* rawPointer = nullptr) : m_Instance{rawPointer} {}

        ~UniqueRef()
        {
            Reset();
        }

        // Delete Copy operations to enforce unique ownership.
        UniqueRef(const UniqueRef&) = delete;
        UniqueRef& operator=(const UniqueRef&) = delete;

        //Move constructor: Transfer the ownership from the source to this instance
        UniqueRef(UniqueRef&& other) noexcept : m_Instance{std::exchange(other.m_Instance, nullptr)} {}

        // Move assignment: Transfers ownership and handles self-assignment
        UniqueRef& operator=(UniqueRef&& other) noexcept
        {
            if (this != &other)
            {
                Reset();
                m_Instance = std::exchange(other.m_Instance, nullptr);
            }
            return *this;
        }

        // Resets the UniqueRef, deleting managed object and optionally takes ownership of a new Object
        void Reset(BaseType* rawPointer = nullptr)
        {
            if (m_Instance)
            {
                delete m_Instance;
                m_Instance = nullptr;
            }
            m_Instance = rawPointer;
        };

        BaseType* operator->() const{return m_Instance;}
        BaseType &operator*() const { return *m_Instance; }

        explicit operator bool() const {return m_Instance != nullptr;}

        BaseType* Get() const
        {
            return m_Instance;
        }

        BaseType* Release()
        {
            BaseType* _temp = m_Instance;
            m_Instance = nullptr;
            return _temp;
        }

    private:
        BaseType* m_Instance;

    };

    //TODO WeakRef
    template<typename BaseType>
    class WeakRef {
    public:
        WeakRef() : m_Instance{nullptr}, m_WeakCountPtr{nullptr}
        {}

        template <typename DerivedType>
        WeakRef(const SharedRef<DerivedType> &sharedRef) :
            m_Instance{sharedRef.m_Instance}, m_WeakCountPtr{sharedRef.m_weakCount}
        {
            IncrementWeakcount();
        }

        ~WeakRef()
        {
            DecrementWeakcount();
        }

        WeakRef(const WeakRef &other) : m_Instance{other.m_Instance}, m_WeakCountPtr{other.m_WeakCountPtr}
        {
            IncrementWeakcount();
        }

        WeakRef& operator=(const WeakRef& other){
            if (this != &other)
            {
                DecrementWeakcount();
                m_Instance = other.m_Instance;
                m_WeakCountPtr = other.m_WeakCountPtr;
                IncrementWeakcount();
            }
            return *this;
        }

        WeakRef(WeakRef &&other) noexcept :
            m_Instance{std::exchange(other.m_Instance, nullptr)},
            m_WeakCountPtr{std::exchange(other.m_WeakCountPtr, nullptr)}
        {}

        WeakRef& operator=(WeakRef&& other) noexcept
        {
            if (this != &other)
            {
                DecrementWeakcount();
                m_Instance = std::exchange(other.m_Instance, nullptr);
                m_WeakCountPtr = std::exchange(other.m_WeakCountPtr, nullptr);
            }
            return *this;
        }

        SharedRef<BaseType> Lock() const
        {
            if (m_WeakCountPtr && *m_WeakCountPtr > 0)
            {
                return SharedRef<BaseType>(m_Instance);
            }
            return SharedRef<BaseType>(nullptr);
        }

    private:
        BaseType* m_Instance;
        std::atomic<uint32_t>* m_WeakCountPtr;

        void IncrementWeakcount() const
        {
            if (m_WeakCountPtr)
            {
                ++(*m_WeakCountPtr);
            }
        }

        void DecrementWeakcount()
        {
            if (m_WeakCountPtr && --(*m_WeakCountPtr) == 0 && !m_Instance)
            {
                delete m_WeakCountPtr;
            }
        }
    };
}