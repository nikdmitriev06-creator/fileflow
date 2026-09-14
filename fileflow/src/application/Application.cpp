#include "Application.h"

#include "domain/job/Job.h"
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
        workerPool_(queue_, config_.workerCount)
    {
    }

    void Application::run()
    {
        using domain::Job;
        using domain::JobOperation;

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

        for (Job::Id id = 1; id <= 6; ++id) {

            auto job = std::make_shared<Job>(
                id,
                "test.txt",
                JobOperation::CalculateHash
            );

            if (!queue_.push(std::move(job))) {
                infrastructure::logging::Logger::error(
                    "Failed to submit job " +
                    std::to_string(id)
                );

                return;
            }

            infrastructure::logging::Logger::info(
                "Submitted job " +
                std::to_string(id)
            );
        }

        infrastructure::logging::Logger::info(
            "Waiting for jobs..."
        );

        queue_.waitUntilEmpty();

        infrastructure::logging::Logger::info(
            "All jobs completed."
        );
    }

} // namespace fileflow::application