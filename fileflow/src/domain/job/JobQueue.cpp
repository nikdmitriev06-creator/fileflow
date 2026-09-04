#include "JobQueue.h"

namespace fileflow::domain {

    void JobQueue::push(std::shared_ptr<Job> job)
    {
        {
            std::lock_guard<std::mutex> lock(mutex_);

            if (shutdown_) {
                return;
            }

            queue_.push(std::move(job));
        }

        condition_.notify_one();
    }

    std::shared_ptr<Job> JobQueue::pop()
    {
        std::unique_lock<std::mutex> lock(mutex_);

        condition_.wait(lock, [this] {
            return !queue_.empty() || shutdown_;
            });

        if (queue_.empty()) {
            return nullptr;
        }

        auto job = std::move(queue_.front());
        queue_.pop();

        return job;
    }

    void JobQueue::shutdown()
    {
        {
            std::lock_guard<std::mutex> lock(mutex_);
            shutdown_ = true;
        }

        condition_.notify_all();
    }

    std::size_t JobQueue::size() const
    {
        std::lock_guard<std::mutex> lock(mutex_);

        return queue_.size();
    }

} // namespace fileflow::domain