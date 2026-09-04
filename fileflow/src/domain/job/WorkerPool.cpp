#include "WorkerPool.h"

#include <chrono>
#include <iostream>
#include <thread>

namespace fileflow::domain {

    WorkerPool::WorkerPool(JobQueue& queue, std::size_t workerCount)
        : queue_(queue)
    {
        // Заранее выделяем память под worker'ов,
        // чтобы vector не делал лишних reallocations.
        workers_.reserve(workerCount);

        for (std::size_t i = 0; i < workerCount; ++i) {

            // std::jthread запускает функцию в отдельном потоке.
            workers_.emplace_back(
                [this, workerId = i](std::stop_token /*stopToken*/) {
                    workerLoop(workerId);
                }
            );
        }
    }

    WorkerPool::~WorkerPool()
    {
        // Просим очередь прекратить приём новых задач.
        //
        // При этом уже находящиеся в очереди задачи
        // всё равно будут обработаны worker'ами.
        queue_.shutdown();

        // После выхода из destructor body начинается
        // уничтожение members.
        //
        // std::jthread автоматически вызовет request_stop()
        // и дождётся завершения каждого потока.
    }

    void WorkerPool::workerLoop(std::size_t workerId)
    {
        while (true) {

            // Если работы нет, поток будет заблокирован здесь,
            // а не будет бессмысленно загружать CPU.
            auto job = queue_.pop();

            // nullptr означает:
            // shutdown() вызван И очередь больше не содержит задач.
            if (!job) {
                break;
            }

            std::cout
                << "Worker " << workerId
                << " processing job " << job->id()
                << '\n';

            // Пока настоящего FileProcessor ещё нет.
            // Имитируем обработку файла.
            std::this_thread::sleep_for(
                std::chrono::milliseconds(500)
            );

            job->start();

            std::cout
                << "Worker " << workerId
                << " started job " << job->id()
                << '\n';

            std::this_thread::sleep_for(
                std::chrono::milliseconds(500)
            );

            job->complete();

            std::cout
                << "Worker " << workerId
                << " completed job " << job->id()
                << '\n';

            // Сообщаем JobQueue, что эта Job полностью завершена.
            //
            // Это важно для waitUntilEmpty().
            queue_.taskCompleted();
        }

        std::cout
            << "Worker " << workerId
            << " stopped\n";
    }

} // namespace fileflow::domain