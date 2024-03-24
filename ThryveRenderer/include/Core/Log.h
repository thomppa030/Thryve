//
// Created by thomppa on 3/24/24.
//

#ifndef LOG_H
#define LOG_H
#include <string>


#include "IService.h"
#include "spdlog/common.h"

namespace spdlog {
    class logger;
}
namespace Thryve::Core {
    class ILoggingService : public IService {
    public:
        ILoggingService();
        ~ILoggingService() override;

        virtual void LogDebug(const std::string& message) = 0;
        virtual void LogInfo(const std::string& message) = 0;
        virtual void LogWarning(const std::string& message) = 0;
        virtual void LogError(const std::string& message) = 0;
        virtual void LogFatal(const std::string &message) = 0;

        void Init(ServiceConfiguration *configuration) override = 0;
        void ShutDown() override = 0;
    };

    struct LoggingServiceConfiguration : ServiceConfiguration {
        spdlog::level::level_enum LogLevel = spdlog::level::debug; // Default log level
        std::string LogFilePath = "logs/development.log";
        bool ConsoleOutputEnabled = true;
        std::string LogPatternConsole = "[%Y-%m-%d %H:%M:%S] %^[%n] [%l] %v%$"; // Default log pattern
        std::string LogPattern = "[%Y-%m-%d %H:%M:%S] [%l] %v"; // Default log pattern
        size_t MaxFileSize = 1048576 * 5; // 5MB
        size_t MaxFiles = 3; // Rotate past 3 files
    };

    struct DevelopmentLoggerConfiguration : LoggingServiceConfiguration {
    };

    //     1. Development Logging Service
    // Purpose: Tailored for development-time needs, focusing on verbose logging, debug messages, and performance
    // metrics. Features: High verbosity level with detailed debug information. Optional logging of performance metrics
    // (e.g., frame times, memory usage). Configurable to toggle on/off certain types of logs dynamically, useful for
    // performance testing or isolating issues.
    class DevelopmentLogger : public ILoggingService {
    public:
        DevelopmentLogger(const char* loggerName = "DevelopmentLogger");
        ~DevelopmentLogger() override;
        void Init(ServiceConfiguration *configuration) override;
        void ShutDown() override;

        void LogDebug(const std::string &message) override;
        void LogInfo(const std::string &message) override;
        void LogWarning(const std::string &message) override;
        void LogError(const std::string &message) override;
        void LogFatal(const std::string &message) override;

    private:
        const char* m_loggerName;
        std::shared_ptr<spdlog::logger> m_logger;
        LoggingServiceConfiguration *m_config{nullptr};
    };

    struct ValidationLayerLoggerConfiguration : LoggingServiceConfiguration {
    };

    class ValidationLayerLogger : public ILoggingService {
    public:
        ValidationLayerLogger(const char* loggerName = "DevelopmentLogger");
        ~ValidationLayerLogger() override;
        void Init(ServiceConfiguration *configuration) override;
        void ShutDown() override;

        void LogDebug(const std::string &message) override;
        void LogInfo(const std::string &message) override;
        void LogWarning(const std::string &message) override;
        void LogError(const std::string &message) override;
        void LogFatal(const std::string &message) override;

    private:
        const char* m_loggerName;
        std::shared_ptr<spdlog::logger> m_logger;
        LoggingServiceConfiguration *m_config{nullptr};
    };

    //     2. Production Logging Service
    // Purpose: Optimized for production environments, prioritizing error reporting, warning messages, and critical
    // information that supports monitoring and troubleshooting without overwhelming storage or processing capabilities.
    // Features:
    // Lower verbosity, focusing on warnings, errors, and critical information.
    // Integration with external monitoring and alerting tools (e.g., Sentry, Datadog).
    // Efficient file management strategies for log rotation and archival.
    class ProductionLogger : public ILoggingService {
    public:
        void Init(ServiceConfiguration *configuration) override;
        void ShutDown() override;
        void LogDebug(const std::string &message) override;
        void LogInfo(const std::string &message) override;
        void LogWarning(const std::string &message) override;
        void LogError(const std::string &message) override;
        void LogFatal(const std::string &message) override;
        ~ProductionLogger() override;
    };
}

#endif //LOG_H
