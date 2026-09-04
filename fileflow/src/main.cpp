#include <chrono>
#include <iostream>
#include <memory>
#include <thread>

#include "domain/job/Job.h"
#include "domain/job/JobQueue.h"
#include "domain/job/WorkerPool.h"

int main()
{
    using fileflow::domain::Job;
    using fileflow::domain::JobOperation;
    using fileflow::domain::JobQueue;
    using fileflow::domain::WorkerPool;

    std::cout << "FileFlow v0.1.0\n";
    std::cout << "Starting application...\n\n";

    // JobQueue живёт дольше WorkerPool,
    // потому что worker'ы используют эту очередь.
    JobQueue queue;

    // Создаём 3 worker-потока.
    WorkerPool workerPool(queue, 3);

    // Добавляем несколько задач.
    for (Job::Id id = 1; id <= 6; ++id) {
        auto job = std::make_shared<Job>(
            id,
            "file_" + std::to_string(id) + ".jpg",
            JobOperation::CalculateHash
        );

        queue.push(std::move(job));

        std::cout
            << "Submitted job: "
            << id
            << '\n';
    }

    std::cout << "\nAll jobs submitted.\n";

    // Здесь main-поток просто ждёт,
    // пока worker'ы обработают задачи.
    //
    // Позже здесь будет HTTP server,
    // поэтому main() больше не будет заниматься
    // ручным ожиданием задач.
    std::this_thread::sleep_for(
        std::chrono::seconds(3)
    );

    std::cout << "\nStopping FileFlow...\n";

    // При выходе из main:
    //
    // 1. WorkerPool уничтожается.
    // 2. Его destructor вызывает queue.shutdown().
    // 3. Worker'ы просыпаются.
    // 4. Worker'ы завершают свои циклы.
    // 5. std::jthread дожидается их завершения.
    //
    // Это называется RAII-based resource management.

    return 0;
}