#pragma once

#include "Job.h"

#include <condition_variable>
#include <cstddef>
#include <memory>
#include <mutex>
#include <queue>

namespace fileflow::domain {

    class JobQueue {
    public:
        JobQueue() = default;
        ~JobQueue() = default;

        JobQueue(const JobQueue&) = delete;
        JobQueue& operator=(const JobQueue&) = delete;

        void push(std::shared_ptr<Job> job);

        [[nodiscard]]
        std::shared_ptr<Job> pop();

        void shutdown();

        [[nodiscard]]
        std::size_t size() const;

    private:
        mutable std::mutex mutex_;
        std::condition_variable condition_;

        std::queue<std::shared_ptr<Job>> queue_;

        bool shutdown_{ false };
    };

} // namespace fileflow::domain
