#pragma once

#include "domain/job/Job.h"
#include "domain/job/JobQueue.h"
#include "domain/job/WorkerPool.h"

#include "infrastructure/config/Config.h"
#include "infrastructure/http/HttpServer.h"
#include "infrastructure/storage/Storage.h"

#include <memory>
#include <string>
#include <mutex>
#include <unordered_map>
#include <atomic>

namespace fileflow::application {

    class Application {
    public:
        explicit Application(
            infrastructure::config::Config config
        );

        ~Application() = default;

        Application(const Application&) = delete;
        Application& operator=(const Application&) = delete;

        void run();

        void stopHttpServer();

        // Создаёт новую задачу и отправляет её в очередь.
        //
        // HttpServer не работает с JobQueue напрямую.
        // Это сохраняет границу между infrastructure и application слоями.
        [[nodiscard]]
        domain::Job::Id submitJob(
            std::string filename,
            domain::JobOperation operation
        );

        [[nodiscard]]
        std::shared_ptr<domain::Job> findJob(
            domain::Job::Id id
        ) const;

    private:
        infrastructure::config::Config config_;

        infrastructure::storage::Storage storage_;

        infrastructure::http::HttpServer httpServer_;

        domain::JobQueue queue_;

        domain::WorkerPool workerPool_;

        std::atomic<domain::Job::Id> nextJobId_{ 1 };

        std::unordered_map<
            domain::Job::Id,
            std::shared_ptr<domain::Job>
        > jobs_;

        mutable std::mutex jobsMutex_;
    };

}