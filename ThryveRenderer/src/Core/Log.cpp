//
// Created by thomppa on 3/24/24.
//

#include "Core/Log.h"

#include <iostream>

#include "spdlog/sinks/rotating_file_sink.h"
#include "spdlog/sinks/stdout_color_sinks.h"
#include "spdlog/spdlog.h"

namespace Thryve::Core {

    ILoggingService::ILoggingService()  = default;
    ILoggingService::~ILoggingService() = default;


    DevelopmentLogger::DevelopmentLogger(const char* loggerName) : m_loggerName{loggerName} {}
    DevelopmentLogger::~DevelopmentLogger()
    {
        DevelopmentLogger::ShutDown();
    }

    void DevelopmentLogger::Init(ServiceConfiguration *configuration)
    {
        m_config = dynamic_cast<DevelopmentLoggerConfiguration*>(configuration);

        if (m_config)
        {

            auto _consoleSink = std::make_shared<spdlog::sinks::stdout_color_sink_mt>();

            _consoleSink->set_pattern(m_config->LogPatternConsole);

            if (m_config->ConsoleOutputEnabled)
                m_logger = std::make_shared<spdlog::logger>(m_loggerName, _consoleSink);
            else
                m_logger = spdlog::rotating_logger_mt("File_Logger", m_config->LogFilePath, m_config->MaxFileSize, m_config->MaxFiles);

            m_logger->set_level(m_config->LogLevel);
        }
        else
        {
            std::cerr << "No valid Configuration for DevelopmentLogger!\n";
        }
    }

    void DevelopmentLogger::ShutDown()
    {
        m_logger.reset();
    }

    void DevelopmentLogger::LogDebug(const std::string &message)
    {
        m_logger->debug(message);
    }
    void DevelopmentLogger::LogInfo(const std::string &message)
    {
        m_logger->info(message);
    }
    void DevelopmentLogger::LogWarning(const std::string &message)
    {
        m_logger->warn(message);
    }
    void DevelopmentLogger::LogError(const std::string &message)
    {
        m_logger->error(message);
    }
    void DevelopmentLogger::LogFatal(const std::string &message) { m_logger->critical(message); }

    ValidationLayerLogger::ValidationLayerLogger(const char *loggerName) : m_loggerName{loggerName} {}
    ValidationLayerLogger::~ValidationLayerLogger() { ValidationLayerLogger::ShutDown(); }

    void ValidationLayerLogger::Init(ServiceConfiguration *configuration)
    {
        m_config = dynamic_cast<ValidationLayerLoggerConfiguration*>(configuration);

        if (m_config)
        {
            auto _consoleSink = std::make_shared<spdlog::sinks::stdout_color_sink_mt>();
            _consoleSink->set_pattern(m_config->LogPatternConsole);

            if (m_config->ConsoleOutputEnabled)
                m_logger = std::make_shared<spdlog::logger>(m_loggerName, _consoleSink);
            else
                m_logger = spdlog::rotating_logger_mt("File_Logger", m_config->LogFilePath, m_config->MaxFileSize, m_config->MaxFiles);

            m_logger->set_level(m_config->LogLevel);
        }
        else
        {
            std::cerr << "No valid Configuration for ValidationLayerLogger!\n";
        }
    }
    void ValidationLayerLogger::ShutDown()
    {
        m_logger.reset();
    }

    void ValidationLayerLogger::LogDebug(const std::string &message)
    {
        m_logger->debug(message);
    }
    void ValidationLayerLogger::LogInfo(const std::string &message)
    {
        m_logger->info(message);
    }
    void ValidationLayerLogger::LogWarning(const std::string &message)
    {
        m_logger->warn(message);
    }
    void ValidationLayerLogger::LogError(const std::string &message)
    {
        m_logger->error(message);
    }
    void ValidationLayerLogger::LogFatal(const std::string &message)
    {
        m_logger->critical(message);
    }
} // namespace Thryve::Core
