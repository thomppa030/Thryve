//
// Created by thomppa on 3/29/24.
//
#pragma once
#include <chrono>
#include <iostream>
#include <string>
#include <thread>
#include <utility>

#include "IService.h"

namespace Thryve::Core {

    class ProfilingService : public IService {
    public:
        ~ProfilingService() override;
        void Init(ServiceConfiguration *configuration) override;
        void ShutDown() override;

        //TODO implement
    };

    class ScopeProfiler {
    public:
        explicit ScopeProfiler(std::string functionName) :
            m_scopeName{functionName}, m_functionName{std::move(functionName)},m_threadID{std::this_thread::get_id()}, m_start{std::chrono::high_resolution_clock::now()}
        {
        }

        explicit ScopeProfiler(std::string serviceName, std::string functionName) :
            m_scopeName{std::move(serviceName)}, m_functionName{std::move(functionName)},
            m_threadID{std::this_thread::get_id()}, m_start{std::chrono::high_resolution_clock::now()}
        {
        }

        ~ScopeProfiler()
        {
            const auto _end = std::chrono::high_resolution_clock::now();
            auto _duration = std::chrono::duration_cast<std::chrono::microseconds>(_end - m_start).count();

            // TODO Now we either send (async) it or print it out I guess?
            std::cout << "Function: " << m_functionName << " Duration: " << _duration << " microseconds" << "\n";
        }

    private:
        std::string m_scopeName;
        std::string m_functionName;
        std::thread::id m_threadID;
        std::chrono::high_resolution_clock::time_point m_start;
    };
} // namespace Thryve::Core

#ifdef ENABLE_PROFILING
#define PROFILE_SCOPE(operationName) Thryve::Core::ScopeProfiler scopeProfilerInstance{operationName, __func__};
#define PROFILE_FUNCTION() Thryve::Core::ScopeProfiler scopeProfilerFunctionInstance{__func__};
#else
#define PROFILE_SCOPE(operationName)
#define PROFILE_FUNCTION()
#endif

