#include <iostream>
#include <memory>

#include "domain/job/Job.h"
#include "domain/job/JobQueue.h"

int main()
{
    using fileflow::domain::Job;
    using fileflow::domain::JobOperation;
    using fileflow::domain::JobQueue;

    JobQueue queue;

    auto job1 = std::make_shared<Job>(
        1,
        "image.jpg",
        JobOperation::CalculateHash
    );

    auto job2 = std::make_shared<Job>(
        2,
        "document.pdf",
        JobOperation::Compress
    );

    queue.push(job1);
    queue.push(job2);

    std::cout << "Queue size: " << queue.size() << '\n';

    auto firstJob = queue.pop();

    if (firstJob) {
        std::cout << "Processing job: "
            << firstJob->id()
            << '\n';
    }

    auto secondJob = queue.pop();

    if (secondJob) {
        std::cout << "Processing job: "
            << secondJob->id()
            << '\n';
    }

    std::cout << "Queue size: " << queue.size() << '\n';

    queue.shutdown();

    return 0;
}