#include "JobQueue.h"

#include <stdexcept>
#include <utility>

namespace fileflow::domain {

    bool JobQueue::push(std::shared_ptr<Job> job)
    {
        if (!job) {
            throw std::invalid_argument(
                "Cannot push null job"
            );
        }

        {
            std::lock_guard<std::mutex> lock(mutex_);

            // После shutdown Queue больше не принимает работу.
            if (shutdown_) {
                return false;
            }

            queue_.push(std::move(job));

            ++unfinishedJobs_;
        }

        // Будим одного ожидающего worker'а.
        condition_.notify_one();

        return true;
    }

    std::shared_ptr<Job> JobQueue::pop()
    {
        std::unique_lock<std::mutex> lock(mutex_);

        condition_.wait(lock, [this] {
            return !queue_.empty() || shutdown_;
            });

        // Если shutdown активен и очередь уже пуста,
        // worker может завершать работу.
        if (queue_.empty()) {
            return nullptr;
        }

        auto job = std::move(queue_.front());

        queue_.pop();

        return job;
    }

    void JobQueue::taskCompleted()
    {
        {
            std::lock_guard<std::mutex> lock(mutex_);

            // Это защитная проверка от логической ошибки:
            // taskCompleted() нельзя вызывать больше раз,
            // чем push().
            if (unfinishedJobs_ == 0) {
                throw std::logic_error(
                    "taskCompleted() called with no unfinished jobs"
                );
            }

            --unfinishedJobs_;
        }

        // Возможно, другой поток ждёт,
        // пока все задачи завершатся.
        condition_.notify_all();
    }

    void JobQueue::waitUntilEmpty()
    {
        std::unique_lock<std::mutex> lock(mutex_);

        condition_.wait(lock, [this] {
            return unfinishedJobs_ == 0;
            });
    }

    void JobQueue::shutdown()
    {
        {
            std::lock_guard<std::mutex> lock(mutex_);

            shutdown_ = true;
        }

        // Просыпаются все worker'ы, ожидающие новые задачи.
        condition_.notify_all();
    }

    std::size_t JobQueue::size() const
    {
        std::lock_guard<std::mutex> lock(mutex_);

        return queue_.size();
    }

    std::size_t JobQueue::unfinishedJobs() const
    {
        std::lock_guard<std::mutex> lock(mutex_);

        return unfinishedJobs_;
    }

} // namespace fileflow::domain