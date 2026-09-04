#pragma once

#include "Job.h"

#include <condition_variable>
#include <cstddef>
#include <memory>
#include <mutex>
#include <queue>

namespace fileflow::domain {

    /**
     * @brief ѕотокобезопасна€ очередь задач.
     *
     * Queue отвечает за:
     * - добавление задач;
     * - выдачу задач worker'ам;
     * - отслеживание незавершЄнных задач;
     * - корректное завершение работы.
     */
    class JobQueue {
    public:
        JobQueue() = default;
        ~JobQueue() = default;

        // Queue владеет mutex и condition_variable,
        // поэтому копирование такой очереди не имеет смысла.
        JobQueue(const JobQueue&) = delete;
        JobQueue& operator=(const JobQueue&) = delete;

        /**
         * @brief ƒобавл€ет новую задачу в очередь.
         */
        void push(std::shared_ptr<Job> job);

        /**
         * @brief ѕолучает следующую задачу.
         *
         * ≈сли задач нет, worker блокируетс€ до:
         * - по€влени€ новой задачи;
         * - или shutdown().
         *
         * @return «адача или nullptr, если очередь завершает работу.
         */
        [[nodiscard]]
        std::shared_ptr<Job> pop();

        /**
         * @brief —ообщает очереди, что задача полностью обработана.
         *
         * ¬ажно: pop() не означает завершение задачи.
         * Worker сначала забирает Job, обрабатывает его,
         * а затем вызывает taskCompleted().
         */
        void taskCompleted();

        /**
         * @brief Ѕлокирует текущий поток до завершени€ всех задач.
         */
        void waitUntilEmpty();

        /**
         * @brief ќстанавливает очередь.
         *
         * ѕосле shutdown() новые задачи не принимаютс€,
         * а ожидающие worker'ы получают сигнал на завершение.
         */
        void shutdown();

        /**
         * @brief ¬озвращает количество задач, наход€щихс€ непосредственно в очереди.
         */
        [[nodiscard]]
        std::size_t size() const;

    private:
        // «ащищает queue_, unfinishedJobs_ и shutdown_.
        mutable std::mutex mutex_;

        // »спользуетс€ дл€ ожидани€:
        // - по€влени€ новой задачи;
        // - завершени€ всех задач.
        std::condition_variable condition_;

        // —ами задачи, ожидающие обработки.
        std::queue<std::shared_ptr<Job>> queue_;

        //  оличество задач, которые ещЄ не завершены.
        //
        // Ќапример:
        //
        // queue:       [Job 3, Job 4]
        // processing:  Job 1, Job 2
        //
        // unfinishedJobs_ == 4
        //
        // ѕоэтому waitUntilEmpty() не завершитс€,
        // пока workers не обработают Job 1-4.
        std::size_t unfinishedJobs_{ 0 };

        // ѕосле shutdown новые задачи добавл€ть нельз€.
        bool shutdown_{ false };
    };

} // namespace fileflow::domain