//
// Created by thomppa on 3/29/24.
//

#include "Core/Profiling.h"

#include <Config.h>
#include <fstream>
#include <nlohmann/json.hpp>

Thryve::Core::ProfilingService::~ProfilingService()
{
    ProfilingService::ShutDown();
}

void Thryve::Core::ProfilingService::Init(ServiceConfiguration *configuration)
{
    IService::Init(configuration);
}

void Thryve::Core::ProfilingService::ShutDown()
{
    auto _filePath = std::string(RESOURCE_DIR)+"/profile_Data.json";
    SaveProfileResultsToJson(_filePath);
}

void Thryve::Core::ProfilingService::RecordProfileResult(const ProfilingData &data)
{
    std::lock_guard _lock(m_mutex);
    m_Profiles[data.Name][data.ThreadID].push_back(data);
}

void Thryve::Core::ProfilingService::SaveProfileResultsToJson(std::string &filePath)
{
    nlohmann::json _json;

    for (const auto& [functionName , threadProfiles] : m_Profiles)
    {
        for (const auto& [threadID, dataVector] : threadProfiles)
        {
            std::ostringstream _oss;
            _oss << threadID;
            std::string _threadIdString = _oss.str();

            nlohmann::json _threadJson;
            for (const auto& _data : dataVector)
            {
                _threadJson.push_back({
                    {"startTime", _data.StartTime},
                    {"duration", _data.Duration}
                });
            }

            _json[functionName][_threadIdString]["invocations"] = _threadJson;
        }
    }
    if (std::ofstream _file(filePath); _file.is_open())
    {
        _file << _json.dump(4);
    }

    std::cout << "JsonFile created" << std::endl;
}
