#pragma once

#include "JobQueue.h"

#include <cstddef>
#include <memory>
#include <thread>
#include <vector>
#include <string>

namespace fileflow::domain {

    /**
     * @brief Набор worker-потоков, обрабатывающих задачи из JobQueue.
     *
     * WorkerPool отвечает только за управление потоками.
     * Сама очередь задач находится в JobQueue.
     */

    class WorkerPool {
    public:
        /**
         * @param queue Очередь, из которой worker'ы будут брать задачи.
         * @param workerCount Количество worker-потоков.
         */
        WorkerPool(JobQueue& queue, std::size_t workerCount);

        // Останавливаем worker'ы и дожидаемся их завершения.
        ~WorkerPool();

        // WorkerPool не должен копироваться:
        // внутри находятся потоки, которыми нельзя безопасно делиться
        // простым копированием объекта.
        WorkerPool(const WorkerPool&) = delete;
        WorkerPool& operator=(const WorkerPool&) = delete;

    private:
        // Функция, которую выполняет каждый worker-поток.
        void workerLoop(std::size_t workerId);

        // Ссылка на очередь принадлежит не WorkerPool.
        // WorkerPool только использует её.
        JobQueue& queue_;

        // Храним все worker-потоки.
        std::vector<std::jthread> workers_;
    };

} // namespace fileflow::domain
