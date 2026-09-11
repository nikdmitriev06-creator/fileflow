#include "Logger.h"

#include <spdlog/sinks/stdout_color_sinks.h>
#include <spdlog/spdlog.h>

#include <memory>
#include <mutex>
#include <stdexcept>
#include <string>

namespace fileflow::infrastructure::logging {

    namespace {

        std::shared_ptr<spdlog::logger>& applicationLogger()
        {
            // static означает, что logger создаётся один раз
            // и живёт до завершения программы.
            static std::shared_ptr<spdlog::logger> logger;

            return logger;
        }

        std::once_flag initializationFlag;

    } // namespace

    void Logger::initialize()
    {
        std::call_once(initializationFlag, [] {

            auto logger =
                spdlog::stdout_color_mt("fileflow");

            // Формат:
            //
            // [2026-09-06 17:20:15.123] [info] Application started
            //
            logger->set_pattern(
                "[%Y-%m-%d %H:%M:%S.%e] [%l] %v"
            );

            logger->set_level(
                spdlog::level::info
            );

            applicationLogger() = std::move(logger);
            });
    }

    void Logger::info(std::string_view message)
    {
        if (!applicationLogger()) {
            throw std::logic_error(
                "Logger has not been initialized"
            );
        }

        applicationLogger()->info(
            "{}",
            message
        );
    }

    void Logger::warning(std::string_view message)
    {
        if (!applicationLogger()) {
            throw std::logic_error(
                "Logger has not been initialized"
            );
        }

        applicationLogger()->warn(
            "{}",
            message
        );
    }

    void Logger::error(std::string_view message)
    {
        if (!applicationLogger()) {
            throw std::logic_error(
                "Logger has not been initialized"
            );
        }

        applicationLogger()->error(
            "{}",
            message
        );
    }

} // namespace fileflow::infrastructure::logging