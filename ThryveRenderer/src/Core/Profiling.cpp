//
// Created by thomppa on 3/29/24.
//

#include "Core/Profiling.h"

#include <Config.h>
#include <fstream>
#include <future>
#include <nlohmann/json.hpp>
#include "Vulkan/VulkanContext.h"



Thryve::Core::ProfilingService::~ProfilingService()
{
    ProfilingService::ShutDown();
}

void Thryve::Core::ProfilingService::Init(ServiceConfiguration *configuration)
{
    IService::Init(configuration);
}

std::string GetNextFileName(const std::string& baseFileName)
{
    static int fileIndex = 0;

    std::ifstream indexFile("index.txt");
    if (indexFile.is_open())
    {
        indexFile >> fileIndex;
        indexFile.close();
    }

    fileIndex++;

    std::ofstream outFile("index.txt");
    {
        if (outFile.is_open())
        {
            outFile << fileIndex;
            outFile.close();
        }
    }

    std::ostringstream fileName;
    fileName << baseFileName << "_" << std::setw(4) << std::setfill('0') << fileIndex << ".json";
    return fileName.str();
}

void Thryve::Core::ProfilingService::ShutDown()
{
    auto _filePath = GetNextFileName(std::string(PROFILE_DIR)+"/profile_Data");
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

    //TODO write a SystemSpecs Static Class that holds this kind of Information, no need to couple to Vulkan here for the GPU!


    nlohmann::json _systemInfo;

    AppSpecification _specs = App::GetAppSpecification();

    _systemInfo.push_back({{"OS", _specs.OSName},
                           {"GPU", _specs.GPU},
                           {"GPU Type", _specs.GPUType},
                           {"CPU", _specs.CPU},
                           {"Core Count", _specs.CPUCoreCount},
                           {"RAM", _specs.RAM}
                  });

    _json["System"] = _systemInfo;

    if (std::ofstream _file(filePath); _file.is_open())
    {
        _file << _json.dump(4);
    }

    std::cout << "JsonFile created" << std::endl;
}


