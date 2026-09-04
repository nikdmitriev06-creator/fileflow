#include <iostream>
#include <memory>

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

    // Queue должна жить дольше WorkerPool,
    // потому что worker'ы используют её во время работы.
    JobQueue queue;

    // Запускаем три worker-потока.
    WorkerPool workerPool(queue, 3);

    // Добавляем шесть задач.
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

    // Ждём, пока ВСЕ задачи будут завершены.
    //
    // Это принципиально отличается от:
    //
    // sleep_for(3 seconds)
    //
    // Здесь программа ждёт ровно столько,
    // сколько действительно требуется worker'ам.
    queue.waitUntilEmpty();

    std::cout << "\nAll jobs completed.\n";

    // WorkerPool уничтожится автоматически при выходе
    // из main().
    //
    // Его destructor вызовет queue.shutdown(),
    // после чего worker'ы завершат свои циклы.
    return 0;
}