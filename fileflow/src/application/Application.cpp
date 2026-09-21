#include "Application.h"

#include "infrastructure/logging/Logger.h"

#include <memory>
#include <string>
#include <utility>
#include <filesystem>
#include <cstddef>
#include <cstring>
#include <stdexcept>
#include <vector>

namespace fileflow::application {

    Application::Application(
        infrastructure::config::Config config
    )
        : config_(std::move(config)),
        storage_(config_.storageDirectory),
        httpServer_(config_.httpPort, *this),
        workerPool_(queue_, config_.workerCount)
    {
    }

    domain::Job::Id Application::submitJob(
        std::string filename,
        domain::JobOperation operation
    )
    {
        const auto jobId = nextJobId_++;

        auto job = std::make_shared<domain::Job>(
            jobId,
            std::move(filename),
            operation
        );

        {
            std::lock_guard lock(jobsMutex_);

            jobs_.emplace(jobId, job);
        }

        if (!queue_.push(job)) {
            std::lock_guard lock(jobsMutex_);
            jobs_.erase(jobId);

            throw std::runtime_error(
                "Failed to submit job: queue is shutting down"
            );
        }

        infrastructure::logging::Logger::info(
            "Submitted job " +
            std::to_string(jobId)
        );

        return jobId;
    }

    std::shared_ptr<domain::Job> Application::findJob(
        domain::Job::Id id
    ) const
    {
        std::lock_guard lock(jobsMutex_);

        const auto it = jobs_.find(id);

        if (it == jobs_.end()) {
            return nullptr;
        }

        return it->second;
    }

    domain::Job::Id Application::submitJob(
        std::string filename,
        std::filesystem::path inputPath,
        domain::JobOperation operation
    )
    {
        const auto jobId = nextJobId_.fetch_add(1);

        auto job = std::make_shared<domain::Job>(
            jobId,
            std::move(filename),
            std::move(inputPath),
            operation
        );

        {
            std::lock_guard lock(jobsMutex_);
            jobs_.emplace(jobId, job);
        }

        if (!queue_.push(job)) {
            std::lock_guard lock(jobsMutex_);
            jobs_.erase(jobId);

            throw std::runtime_error(
                "Failed to submit job: queue is shutting down"
            );
        }

        infrastructure::logging::Logger::info(
            "Submitted job " + std::to_string(jobId)
        );

        return jobId;
    }

    domain::Job::Id Application::uploadFile(
        std::string filename,
        const std::string& content,
        domain::JobOperation operation
    )
    {
        if (filename.empty()) {
            throw std::invalid_argument(
                "Filename must not be empty"
            );
        }

        // Ограничиваем размер имени файла.
        if (filename.size() > 255) {
            throw std::invalid_argument(
                "Filename is too long"
            );
        }

        if (content.size() > config_.maxUploadSizeBytes) {
            throw std::invalid_argument(
                "File is too large"
            );
        }

        const auto jobId = nextJobId_.fetch_add(1);

        /*
         * Не используем исходное имя файла как имя внутри storage.
         *
         * Клиент может прислать что-то вроде:
         *
         *     ../../secret.txt
         *
         * Поэтому физическое имя генерируем сами.
         *
         * Например:
         *
         *     42.txt
         *
         * А оригинальное имя "photo.txt" остаётся только
         * в metadata Job.
         */
        const auto extension =
            std::filesystem::path(filename).extension().string();

        const auto storedFilename =
            std::to_string(jobId) + extension;

        const auto inputPath =
            std::filesystem::absolute(
                storage_.rootDirectory() / storedFilename
            );

        /*
         * cpp-httplib отдаёт multipart content как std::string.
         *
         * std::string здесь безопасен даже для бинарных данных:
         * он может содержать '\0'.
         *
         * Storage работает с vector<std::byte>, поэтому
         * делаем явное байтовое копирование.
         */
        std::vector<std::byte> data(content.size());

        std::memcpy(
            data.data(),
            content.data(),
            content.size()
        );

        try {
            storage_.save(
                storedFilename,
                data
            );

            auto job = std::make_shared<domain::Job>(
                jobId,
                std::move(filename),
                inputPath,
                operation
            );

            {
                std::lock_guard lock(jobsMutex_);
                jobs_.emplace(jobId, job);
            }

            if (!queue_.push(job)) {
                std::lock_guard lock(jobsMutex_);
                jobs_.erase(jobId);

                // Если очередь уже закрывается, файл нам больше не нужен.
                storage_.remove(storedFilename);

                throw std::runtime_error(
                    "Failed to submit job: queue is shutting down"
                );
            }

            infrastructure::logging::Logger::info(
                "Uploaded file for job " +
                std::to_string(jobId)
            );

            return jobId;
        }
        catch (...) {
            /*
             * Если после сохранения файла произошла ошибка
             * при создании Job или постановке в очередь,
             * не оставляем orphan-файл в storage.
             */
            storage_.remove(storedFilename);
            throw;
        }
    }

    void Application::run()
    {
        infrastructure::logging::Logger::info(
            "FileFlow v0.1.0"
        );

        infrastructure::logging::Logger::info(
            "Starting application..."
        );

        infrastructure::logging::Logger::info(
            "Worker count: " +
            std::to_string(config_.workerCount)
        );

        infrastructure::logging::Logger::info(
            "Storage directory: " +
            config_.storageDirectory
        );

        httpServer_.run();
    }

    void Application::stopHttpServer()
    {
        httpServer_.stop();
    }

}