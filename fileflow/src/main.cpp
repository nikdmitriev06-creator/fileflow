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

    // Queue должна существовать дольше WorkerPool,
    // потому что worker'ы используют её.
    JobQueue queue;

    // Запускаем 3 worker-потока.
    WorkerPool workerPool(queue, 3);

    // Создаём несколько задач для одного файла.
    for (Job::Id id = 1; id <= 6; ++id) {

        auto job = std::make_shared<Job>(
            id,
            "test.txt",
            JobOperation::CalculateHash
        );

        queue.push(std::move(job));

        std::cout
            << "Submitted job: "
            << id
            << '\n';
    }

    std::cout << "\nWaiting for jobs...\n\n";

    // Ждём фактического завершения всех задач.
    queue.waitUntilEmpty();

    std::cout << "\nAll jobs completed.\n";

    // При выходе:
    //
    // 1. WorkerPool destructor вызывает shutdown().
    // 2. Worker'ы просыпаются.
    // 3. Worker'ы выходят из workerLoop().
    // 4. std::jthread дожидается их завершения.
    return 0;
}