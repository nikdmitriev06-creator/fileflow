#include "domain/job/Job.h"
#include "domain/job/JobQueue.h"

#include <cassert>
#include <iostream>
#include <memory>
#include <stdexcept>

using fileflow::domain::Job;
using fileflow::domain::JobOperation;
using fileflow::domain::JobQueue;
using fileflow::domain::JobStatus;

void testJobInitialState()
{
    Job job(
        1,
        "test.txt",
        JobOperation::CalculateHash
    );

    assert(job.id() == 1);
    assert(job.filename() == "test.txt");
    assert(job.operation() == JobOperation::CalculateHash);

    // Новая задача всегда начинается с Pending.
    assert(job.status() == JobStatus::Pending);

    assert(!job.startedAt().has_value());
    assert(!job.completedAt().has_value());
    assert(!job.error().has_value());
    assert(!job.result().has_value());
}

void testJobSuccessfulFlow()
{
    Job job(
        1,
        "test.txt",
        JobOperation::CalculateHash
    );

    job.start();

    assert(job.status() == JobStatus::Processing);
    assert(job.startedAt().has_value());

    job.complete("abcdef123456");

    assert(job.status() == JobStatus::Completed);
    assert(job.completedAt().has_value());
    assert(job.result().has_value());
    assert(*job.result() == "abcdef123456");
}

void testJobFailedFlow()
{
    Job job(
        1,
        "missing.txt",
        JobOperation::CalculateHash
    );

    job.start();

    job.fail("Failed to open file");

    assert(job.status() == JobStatus::Failed);
    assert(job.completedAt().has_value());
    assert(job.error().has_value());
    assert(*job.error() == "Failed to open file");
}

void testInvalidJobTransition()
{
    Job job(
        1,
        "test.txt",
        JobOperation::CalculateHash
    );

    bool exceptionThrown = false;

    try {
        // Нельзя завершить задачу,
        // которая ещё не начала выполняться.
        job.complete("result");
    }
    catch (const std::logic_error&) {
        exceptionThrown = true;
    }

    assert(exceptionThrown);
}

void testQueuePushAndPop()
{
    JobQueue queue;

    auto job = std::make_shared<Job>(
        1,
        "test.txt",
        JobOperation::CalculateHash
    );

    assert(queue.push(job));

    assert(queue.size() == 1);
    assert(queue.unfinishedJobs() == 1);

    auto receivedJob = queue.pop();

    assert(receivedJob != nullptr);
    assert(receivedJob->id() == 1);

    // Job уже находится у worker,
    // поэтому queue.size() == 0.
    assert(queue.size() == 0);

    // Но задача всё ещё не завершена.
    assert(queue.unfinishedJobs() == 1);

    queue.taskCompleted();

    assert(queue.unfinishedJobs() == 0);
}

void testQueueRejectsJobsAfterShutdown()
{
    JobQueue queue;

    queue.shutdown();

    auto job = std::make_shared<Job>(
        1,
        "test.txt",
        JobOperation::CalculateHash
    );

    // После shutdown новые задачи не принимаются.
    assert(!queue.push(job));

    assert(queue.size() == 0);
    assert(queue.unfinishedJobs() == 0);
}

void testQueueRejectsNullJob()
{
    JobQueue queue;

    bool exceptionThrown = false;

    try {
        queue.push(nullptr);
    }
    catch (const std::invalid_argument&) {
        exceptionThrown = true;
    }

    assert(exceptionThrown);
}

int main()
{
    testJobInitialState();
    testJobSuccessfulFlow();
    testJobFailedFlow();
    testInvalidJobTransition();

    testQueuePushAndPop();
    testQueueRejectsJobsAfterShutdown();
    testQueueRejectsNullJob();

    std::cout << "All tests passed.\n";

    return 0;
}