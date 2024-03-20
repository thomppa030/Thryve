//
// Created by kprie on 20.03.2024.
//
#pragma once

#include <atomic>
#include <unordered_set>
#include <utility>
#include <mutex>

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
            ++m_ReferenceCount;
        }
        void DecrementReferenceCount() const {
            --m_ReferenceCount;
        }

        uint32_t GetReferenceCount() const {return m_ReferenceCount.load();}

    private:
        mutable std::atomic<uint32_t> m_ReferenceCount{0};
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
            if (this != &other)
            {
                m_Instance = static_cast<BaseType*>(other.m_Instance);
                IncrementReferenceCount();
            }
        }

        template <typename DerivedType>
        SharedRef(const SharedRef<DerivedType>&& other) {
            m_Instance = (BaseType*)other.m_Instance;
        }

        /**
         * @brief Creates a new SharedRef object by copying the m_Instance from another SharedRef object without incrementing the reference count.
         *
         * This method creates a new SharedRef object by copying the m_Instance member from the provided 'other' SharedRef object, without incrementing the reference count.
         *
         * @param other The SharedRef object from which the m_Instance should be copied.
         * @return A new SharedRef object with the copied m_Instance from the 'other' SharedRef object.
         *
         * @note The reference count of the m_Instance in the returned SharedRef object is not incremented.
         */
        static SharedRef<BaseType> CopyWithoutIncrement(const SharedRef<BaseType>& other) {
            SharedRef<BaseType> _result = nullptr;
            _result.m_Instance = other.m_Instance;
            return _result;
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
            if (other.m_Instance != m_Instance)
            {
                other.IncrementReferenceCount();
                DecrementReferenceCount();
                m_Instance = other.m_Instance;
            }
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
            if (m_Instance != instance)
            {
                DecrementReferenceCount();
                m_Instance = instance;
            }
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

        template<class DerivedType>
        friend class SharedRef;

        mutable BaseType* m_Instance;
    };

    //TODO WeakRef
}