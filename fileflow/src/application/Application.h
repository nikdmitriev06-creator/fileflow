#pragma once

#include "domain/job/JobQueue.h"
#include "domain/job/WorkerPool.h"

#include "infrastructure/config/Config.h"
#include "infrastructure/storage/Storage.h"

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

    private:
        infrastructure::config::Config config_;

        infrastructure::storage::Storage storage_;

        // Queue должна существовать дольше WorkerPool,
        // потому что WorkerPool хранит ссылку на неё.
        domain::JobQueue queue_;

        domain::WorkerPool workerPool_;
    };

} // namespace fileflow::application
