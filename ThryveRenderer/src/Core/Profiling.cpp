//
// Created by thomppa on 3/29/24.
//

#include "Core/Profiling.h"

#include <Config.h>
#include <fstream>
#include <future>
#include <nlohmann/json.hpp>
#include "Vulkan/VulkanContext.h"

#ifdef _WIN32
#include <windows.h>
std::string GetOSName() {
    return "Windows";
}
#elif __linux__
#include <fstream>
#include <sstream>
std::string GetOSName() {
    std::ifstream file("/etc/os-release");
    std::string line;
    while (std::getline(file, line)) {
        std::istringstream iss(line);
        std::string key, value;
        if (std::getline(std::getline(iss, key, '='), value)) {
            if (key == "PRETTY_NAME") {
                // Remove leading and trailing quotes
                if (value.size() >= 2 && value.front() == '"' && value.back() == '"') {
                    value = value.substr(1, value.size() - 2);
                }
                return value;
            }
        }
    }
    return "Unknown Linux distribution";
}
#elif __APPLE__
#include <array>
#include <memory>
std::string GetOSName() {
    std::array<char, 128> buffer;
    std::string result;
    std::unique_ptr<FILE, decltype(&pclose)> pipe(popen("sw_vers -productName", "r"), pclose);
    if (!pipe) {
        return "Unknown macOS version";
    }
    while (fgets(buffer.data(), buffer.size(), pipe.get()) != nullptr) {
        result += buffer.data();
    }
    // Remove the newline character
    result.erase(remove(result.begin(), result.end(), '\n'), result.end());
    return result;
}
#else
std::string GetOSName() {
    return "Unsupported OS";
}
#endif

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

struct SystemSpecs {
    std::string OSName;
    std::string GraphicsCard;
};

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

    SystemSpecs _specs = {};
    _specs.OSName = GetOSName();
    _specs.GraphicsCard = "Nvidia 2080 TI";

    nlohmann::json _systemInfo;

    _systemInfo.push_back({
                      {"OSName", _specs.OSName},
                      {"GraphicsCard", _specs.GraphicsCard}
                  });

    _json["System"] = _systemInfo;

    if (std::ofstream _file(filePath); _file.is_open())
    {
        _file << _json.dump(4);
    }

    std::cout << "JsonFile created" << std::endl;
}


