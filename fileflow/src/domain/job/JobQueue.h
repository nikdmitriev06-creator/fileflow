#pragma once

#include "Job.h"

#include <condition_variable>
#include <cstddef>
#include <memory>
#include <mutex>
#include <queue>

namespace fileflow::domain {

    /**
     * @brief Потокобезопасная очередь задач.
     *
     * Queue отвечает за:
     * - добавление задач;
     * - выдачу задач worker'ам;
     * - отслеживание незавершённых задач;
     * - корректное завершение работы.
     */
    class JobQueue {
    public:
        JobQueue() = default;
        ~JobQueue() = default;

        JobQueue(const JobQueue&) = delete;
        JobQueue& operator=(const JobQueue&) = delete;

        /**
         * @brief Добавляет задачу в очередь.
         *
         * @return true, если задача была добавлена.
         * @return false, если очередь уже остановлена.
         *
         * @throws std::invalid_argument если job == nullptr.
         */
        [[nodiscard]]
        bool push(std::shared_ptr<Job> job);

        /**
         * @brief Получает следующую задачу.
         *
         * @return Job или nullptr, если очередь завершает работу
         *         и задач больше не осталось.
         */
        [[nodiscard]]
        std::shared_ptr<Job> pop();

        /**
         * @brief Сообщает очереди о завершении задачи.
         */
        void taskCompleted();

        /**
         * @brief Ждёт завершения всех существующих задач.
         */
        void waitUntilEmpty();

        /**
         * @brief Останавливает очередь.
         *
         * Новые задачи после этого не принимаются.
         */
        void shutdown();

        /**
         * @brief Количество задач, ожидающих обработки.
         */
        [[nodiscard]]
        std::size_t size() const;

        /**
         * @brief Количество всех незавершённых задач.
         *
         * Сюда входят:
         * - задачи в queue_;
         * - задачи, которые уже обрабатываются worker'ами.
         */
        [[nodiscard]]
        std::size_t unfinishedJobs() const;

    private:
        mutable std::mutex mutex_;

        std::condition_variable condition_;

        std::queue<std::shared_ptr<Job>> queue_;

        // Количество задач, которые были приняты Queue,
        // но ещё не завершены worker'ами.
        std::size_t unfinishedJobs_{ 0 };

        bool shutdown_{ false };
    };

} // namespace fileflow::domain