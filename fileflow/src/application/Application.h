#pragma once

#include "domain/job/Job.h"
#include "domain/job/JobQueue.h"
#include "domain/job/WorkerPool.h"

#include "infrastructure/config/Config.h"
#include "infrastructure/http/HttpServer.h"
#include "infrastructure/storage/Storage.h"

#include <memory>
#include <string>

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

        // Создаёт новую задачу и отправляет её в очередь.
        //
        // HttpServer не работает с JobQueue напрямую.
        // Это сохраняет границу между infrastructure и application слоями.
        [[nodiscard]]
        domain::Job::Id submitJob(
            std::string filename,
            domain::JobOperation operation
        );

    private:
        infrastructure::config::Config config_;

        infrastructure::storage::Storage storage_;

        infrastructure::http::HttpServer httpServer_;

        // Queue должна существовать дольше WorkerPool,
        // потому что WorkerPool хранит ссылку на неё.
        domain::JobQueue queue_;

        domain::WorkerPool workerPool_;

        // Пока храним только следующий ID в памяти.
        // PostgreSQL появится позже и возьмёт на себя постоянное хранение.
        domain::Job::Id nextJobId_{ 1 };
    };

}