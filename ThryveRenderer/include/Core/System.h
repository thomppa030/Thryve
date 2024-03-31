//
// Created by thomppa on 3/31/24.
//
#pragma once

#ifdef _WIN32
#include <windows.h>
namespace SystemSpecs {
    std::string GetOSName() { return "Windows"; }

    DWORDLONG GetTotalRAM()
    {
        MEMORYSTATUSEX memInfo;
        memInfo.dwLength = sizeof(MEMORYSTATUSEX);
        GlobalMemoryStatusEx(&memInfo);
        DWORDLONG totalRAM = memInfo.ullTotalPhys / (1024 * 1024); // Convert to MB

        return totalRAM;
    }

    std::string GetCPUName()
    {
        HKEY hKey;
        const char *path = "HARDWARE\\DESCRIPTION\\System\\CentralProcessor\\0";
        if (RegOpenKeyExA(HKEY_LOCAL_MACHINE, path, 0, KEY_READ, &hKey) == ERROR_SUCCESS)
        {
            DWORD type = REG_SZ; // String type
            char value[256] = {0};
            DWORD value_length = sizeof(value);
            if (RegQueryValueExA(hKey, "ProcessorNameString", NULL, &type, (LPBYTE)&value, &value_length) ==
                ERROR_SUCCESS)
            {
                RegCloseKey(hKey);
                return std::string(value);
            }
            RegCloseKey(hKey);
        }
        return "Unknown Processor";
    }

    int GetProcessorCoreCount()
    {
        SYSTEM_INFO sysInfo;
        GetSystemInfo(&sysInfo);
        int numCPU = sysInfo.dwNumberOfProcessors;

        return numCPU;
    }
} // namespace myNamespace

#elif __linux__
#include <fstream>
#include <string>
#include <sstream>
namespace SystemSpecs {
    static std::string GetOSName()
    {
        std::ifstream file("/etc/os-release");
        std::string line;
        while (std::getline(file, line))
        {
            std::istringstream iss(line);
            std::string key, value;
            if (std::getline(std::getline(iss, key, '='), value))
            {
                if (key == "PRETTY_NAME")
                {
                    // Remove leading and trailing quotes
                    if (value.size() >= 2 && value.front() == '"' && value.back() == '"')
                    {
                        value = value.substr(1, value.size() - 2);
                    }
                    return value;
                }
            }
        }
        return "Unknown Linux distribution";
    }

    inline std::string TrimSpaces(const std::string &str)
    {
        std::string trimmed = str;

        // Left trim
        trimmed.erase(trimmed.begin(),
                      std::find_if(trimmed.begin(), trimmed.end(), [](unsigned char ch) { return !std::isspace(ch); }));

        // Right trim
        trimmed.erase(
            std::find_if(trimmed.rbegin(), trimmed.rend(), [](unsigned char ch) { return !std::isspace(ch); }).base(),
            trimmed.end());

        return trimmed;
    }

    static std::string GetTotalRAM()
    {
        std::ifstream _meminfo("/proc/meminfo");
        std::string _line;
        while (std::getline(_meminfo, _line))
        {
            if (_line.find("MemTotal:") != std::string::npos)
            {
                return TrimSpaces(_line.substr(_line.find(':') + 1));
            }
            break;
        }
        return "Total RAM could not be detected";
    }

    static std::string GetCPUName()
    {
        std::ifstream cpuinfo("/proc/cpuinfo");
        std::string line;
        while (std::getline(cpuinfo, line))
        {
            if (line.find("model name") != std::string::npos)
            {
                return line.substr(line.find(":") + 2);
            }
        }
        return "Unknown Processor";
    }

    static int GetProcessorCoreCount()
    {
        std::ifstream _cpuinfo("/proc/cpuinfo");
        std::string _line;
        int _numCpu = 0;
        while (std::getline(_cpuinfo, _line)) {
            if (_line.find("processor") != std::string::npos) {
                _numCpu++;
            }
        }
        return _numCpu;
    }

}
#elif __APPLE__
#include <array>
#include <memory>
    std::string GetOSName()
    {
        std::array<char, 128> buffer;
        std::string result;
        std::unique_ptr<FILE, decltype(&pclose)> pipe(popen("sw_vers -productName", "r"), pclose);
        if (!pipe)
        {
            return "Unknown macOS version";
        }
        while (fgets(buffer.data(), buffer.size(), pipe.get()) != nullptr)
        {
            result += buffer.data();
        }
        // Remove the newline character
        result.erase(remove(result.begin(), result.end(), '\n'), result.end());
        return result;
    }

    //TODO implement for Mac
    int GetTotalRAM() {}

    int GetProcessorCoreCount() {}
#else
    std::string GetOSName() { return "Unsupported OS"; }
#endif
