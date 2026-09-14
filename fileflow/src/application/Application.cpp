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

        if (!queue_.push(job)) {
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