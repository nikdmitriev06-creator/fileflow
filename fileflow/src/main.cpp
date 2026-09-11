#include <iostream>
#include <memory>
#include <string>

#include "domain/job/Job.h"
#include "domain/job/JobQueue.h"
#include "domain/job/WorkerPool.h"

#include "infrastructure/config/Config.h"
#include "infrastructure/logging/Logger.h"

int main()
{
    using fileflow::domain::Job;
    using fileflow::domain::JobOperation;
    using fileflow::domain::JobQueue;
    using fileflow::domain::WorkerPool;

    using fileflow::infrastructure::config::loadConfig;
    using fileflow::infrastructure::logging::Logger;

    try {
        // Загружаем настройки до запуска остальных компонентов.
        const auto config = loadConfig("config.json");

        Logger::initialize();

        Logger::info("FileFlow v0.1.0");
        Logger::info("Starting application...");

        Logger::info(
            "Worker count: " +
            std::to_string(config.workerCount)
        );

        Logger::info(
            "Storage directory: " +
            config.storageDirectory
        );

        JobQueue queue;

        // Теперь количество worker'ов задаётся конфигурацией,
        // а не жёстко зашито в коде.
        WorkerPool workerPool(
            queue,
            config.workerCount
        );

        for (Job::Id id = 1; id <= 6; ++id) {

            auto job = std::make_shared<Job>(
                id,
                "test.txt",
                JobOperation::CalculateHash
            );

            if (!queue.push(std::move(job))) {
                Logger::error(
                    "Failed to submit job " +
                    std::to_string(id)
                );

                return 1;
            }

            Logger::info(
                "Submitted job " +
                std::to_string(id)
            );
        }

        Logger::info("Waiting for jobs...");

        queue.waitUntilEmpty();

        Logger::info("All jobs completed.");

        return 0;
    }
    catch (const std::exception& error) {

        // Ошибка на этапе запуска приложения.
        //
        // Например:
        // - config.json отсутствует;
        // - JSON повреждён;
        // - worker_count == 0.
        //
        // Logger здесь может быть ещё не инициализирован,
        // поэтому используем std::cerr.
        std::cerr
            << "Fatal error: "
            << error.what()
            << '\n';

        return 1;
    }
}