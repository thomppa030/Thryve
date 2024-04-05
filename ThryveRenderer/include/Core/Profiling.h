//
// Created by thomppa on 3/29/24.
//
#pragma once
 #include <chrono>
#include <iostream>
#include <string>
#include <thread>
#include <unordered_map>
#include "IService.h"
#include "ServiceRegistry.h"



namespace Thryve::Core {
    struct ProfileKey {
        std::string Name;
        std::string ScopeName;

        bool operator==(const ProfileKey& other) const {
            return Name == other.Name && ScopeName == other.ScopeName;
        }

        std::size_t operator()(const Thryve::Core::ProfileKey& k) const {
            return std::hash<std::string>()(k.Name) ^ (std::hash<std::string>()(k.ScopeName) << 1);
        }
    };

    struct ProfilingData {
        std::string Name;
        std::string ScopeName;
        std::thread::id ThreadID;
        std::time_t StartTime;
        long long Duration;
    };
}

namespace std {
    template <>
    struct hash<Thryve::Core::ProfileKey> {
        size_t operator()(const Thryve::Core::ProfileKey& k) const noexcept {
            return hash<string>()(k.Name) ^ (hash<string>()(k.ScopeName) << 1);
        }
    };
}

namespace Thryve::Core
{
    class ProfilingService : public IService {
    public:
        ~ProfilingService() override;
        void Init(ServiceConfiguration *configuration) override;
        void ShutDown() override;

        void RecordProfileResult(const ProfilingData& data);

        void SaveProfileResultsToJson(std::string& filePath);
    private:
        std::mutex m_mutex;
        std::unordered_map<ProfileKey, std::unordered_map<std::thread::id, std::vector<ProfilingData>>> m_Profiles;
    };

    class ScopeProfiler {
    public:
        explicit ScopeProfiler(const std::string& functionName)
        : m_Data{functionName, "",std::this_thread::get_id()}
        , m_start{std::chrono::steady_clock::now()}
        {

        }

        explicit ScopeProfiler(const std::string& scopeName, const std::string& functionName) :
          m_Data{functionName,scopeName, std::this_thread::get_id()}
        , m_start{std::chrono::steady_clock::now()}
        {
        }

        ~ScopeProfiler()
        {
            const auto _end = std::chrono::steady_clock::now();
            m_Data.Duration = std::chrono::duration_cast<std::chrono::microseconds>(_end - m_start).count();
            ServiceRegistry::GetService<ProfilingService>()->RecordProfileResult(m_Data);
        }

    private:
        ProfilingData m_Data;
        std::chrono::steady_clock::time_point m_start;
    };
} // namespace Thryve::Core



#ifdef ENABLE_PROFILING
#define PROFILE_SCOPE(operationName) Thryve::Core::ScopeProfiler scopeProfilerInstance{operationName, __func__};
#define PROFILE_FUNCTION() Thryve::Core::ScopeProfiler scopeProfilerFunctionInstance{__func__};
#else
#define PROFILE_SCOPE(operationName)
#define PROFILE_FUNCTION()
#endif
