#include "JobQueue.h"

#include <utility>

namespace fileflow::domain {

    void JobQueue::push(std::shared_ptr<Job> job)
    {
        {
            // Блокируем очередь на время изменения
            // её внутреннего состояния.
            std::lock_guard<std::mutex> lock(mutex_);

            // После shutdown новые задачи не принимаем.
            if (shutdown_) {
                return;
            }

            queue_.push(std::move(job));

            // Задача считается незавершённой до тех пор,
            // пока worker явно не вызовет taskCompleted().
            ++unfinishedJobs_;
        }

        // Будим один worker, ожидающий появления работы.
        condition_.notify_one();
    }

    std::shared_ptr<Job> JobQueue::pop()
    {
        std::unique_lock<std::mutex> lock(mutex_);

        // Если очередь пустая, worker засыпает.
        //
        // Важно использовать predicate:
        // condition_variable может проснуться ложно
        // (spurious wakeup).
        condition_.wait(lock, [this] {
            return !queue_.empty() || shutdown_;
            });

        // Если очередь пуста и shutdown активирован,
        // значит новых задач больше не будет.
        if (queue_.empty()) {
            return nullptr;
        }

        // Перемещаем Job из очереди в worker.
        auto job = std::move(queue_.front());

        queue_.pop();

        return job;
    }

    void JobQueue::taskCompleted()
    {
        {
            std::lock_guard<std::mutex> lock(mutex_);

            // Worker закончил обработку одной задачи.
            --unfinishedJobs_;
        }

        // Будим поток, который мог ждать завершения
        // всех задач через waitUntilEmpty().
        condition_.notify_all();
    }

    void JobQueue::waitUntilEmpty()
    {
        std::unique_lock<std::mutex> lock(mutex_);

        // Ждём именно unfinishedJobs_ == 0.
        //
        // Проверять queue_.empty() было бы неправильно:
        // worker мог уже забрать Job из queue_,
        // но всё ещё обрабатывать его.
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

        // Все worker'ы, которые сейчас спят внутри pop(),
        // должны проснуться и проверить shutdown_.
        condition_.notify_all();
    }

    std::size_t JobQueue::size() const
    {
        std::lock_guard<std::mutex> lock(mutex_);

        return queue_.size();
    }

} // namespace fileflow::domain