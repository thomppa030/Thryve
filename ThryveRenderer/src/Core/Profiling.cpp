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
    m_Profiles[ProfileKey{data.Name, data.ScopeName}][data.ThreadID].push_back(data);
}

bool has_non_ascii(const std::string& str) {
    return std::any_of(str.begin(), str.end(), [](unsigned char c) {
                           return c >= 128; // Non-ASCII characters are >= 128
                       });
}

void check_json(const nlohmann::json& j) {
    if (j.is_string()) {
        if (has_non_ascii(j.get<std::string>())) {
            std::cout << "Found non-ASCII string: " << j.get<std::string>() << std::endl;
        }
    } else if (j.is_object() || j.is_array()) {
        for (auto& el : j.items()) {
            check_json(el.value());
        }
    }
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

            _json[functionName.Name][functionName.ScopeName][_threadIdString]["invocations"] = _threadJson;
        }
    }

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
        try {
            if (std::ofstream _file(filePath); _file.is_open()) {
                _file << _json.dump(4);
                std::cout << "JsonFile created successfully." << std::endl;
            } else {
                throw std::runtime_error("Unable to open file for writing.");
            }
        } catch (const std::exception& e) {
            std::cerr << "Exception occurred: " << e.what() << std::endl;
            // Handle exception (e.g., cleanup or retry logic)
        }
    }
}


