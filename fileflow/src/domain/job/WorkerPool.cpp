#include "WorkerPool.h"

#include <chrono>
#include <iostream>
#include <utility>

namespace fileflow::domain {

    WorkerPool::WorkerPool(JobQueue& queue, std::size_t workerCount)
        : queue_(queue)
    {
        // Создаём указанное количество worker-потоков.
        workers_.reserve(workerCount);

        for (std::size_t i = 0; i < workerCount; ++i) {
            // std::jthread автоматически запускает переданную функцию
            // в отдельном потоке.
            workers_.emplace_back(
                [this, workerId = i](std::stop_token stopToken) {
                    workerLoop(workerId);
                }
            );
        }
    }

    WorkerPool::~WorkerPool()
    {
        // Сообщаем очереди, что новые задачи больше не принимаются
        // и ожидающие worker'ы должны завершиться.
        queue_.shutdown();

        // std::jthread автоматически дождётся завершения всех потоков.
    }

    void WorkerPool::workerLoop(std::size_t workerId)
    {
        while (true) {
            // pop() блокирует поток, если очередь пуста.
            //
            // Если очередь была остановлена и задач больше нет,
            // pop() вернёт nullptr.
            auto job = queue_.pop();

            if (!job) {
                // Работа закончена.
                break;
            }

            std::cout
                << "Worker " << workerId
                << " processing job " << job->id()
                << '\n';

            // На данном этапе настоящей обработки файла ещё нет.
            // Имитируем работу worker'а.
            std::this_thread::sleep_for(
                std::chrono::milliseconds(500)
            );

            job->start();

            std::cout
                << "Worker " << workerId
                << " started job " << job->id()
                << '\n';

            // Ещё немного имитируем обработку.
            std::this_thread::sleep_for(
                std::chrono::milliseconds(500)
            );

            job->complete();

            std::cout
                << "Worker " << workerId
                << " completed job " << job->id()
                << '\n';
        }

        std::cout
            << "Worker " << workerId
            << " stopped\n";
    }

} // namespace fileflow::domain