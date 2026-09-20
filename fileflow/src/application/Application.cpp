#include "Application.h"

#include "infrastructure/logging/Logger.h"

#include <memory>
#include <string>
#include <utility>

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

}