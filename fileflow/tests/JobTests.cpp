#include "domain/job/Job.h"
#include "domain/job/JobQueue.h"

#include <gtest/gtest.h>

#include <memory>

using fileflow::domain::Job;
using fileflow::domain::JobOperation;
using fileflow::domain::JobQueue;
using fileflow::domain::JobStatus;

TEST(JobTest, InitialState)
{
    const Job job(
        1,
        "test.txt",
        JobOperation::CalculateHash
    );

    EXPECT_EQ(job.id(), 1);
    EXPECT_EQ(job.filename(), "test.txt");
    EXPECT_EQ(
        job.operation(),
        JobOperation::CalculateHash
    );

    EXPECT_EQ(
        job.status(),
        JobStatus::Pending
    );

    EXPECT_FALSE(job.startedAt().has_value());
    EXPECT_FALSE(job.completedAt().has_value());
    EXPECT_FALSE(job.error().has_value());
    EXPECT_FALSE(job.result().has_value());
}

TEST(JobTest, SuccessfulFlow)
{
    Job job(
        1,
        "test.txt",
        JobOperation::CalculateHash
    );

    job.start();

    EXPECT_EQ(
        job.status(),
        JobStatus::Processing
    );

    EXPECT_TRUE(job.startedAt().has_value());

    job.complete("abcdef123456");

    EXPECT_EQ(
        job.status(),
        JobStatus::Completed
    );

    EXPECT_TRUE(job.completedAt().has_value());
    EXPECT_TRUE(job.result().has_value());

    EXPECT_EQ(
        *job.result(),
        "abcdef123456"
    );
}

TEST(JobTest, FailedFlow)
{
    Job job(
        1,
        "missing.txt",
        JobOperation::CalculateHash
    );

    job.start();

    job.fail("Failed to open file");

    EXPECT_EQ(
        job.status(),
        JobStatus::Failed
    );

    EXPECT_TRUE(job.completedAt().has_value());
    EXPECT_TRUE(job.error().has_value());

    EXPECT_EQ(
        *job.error(),
        "Failed to open file"
    );
}

TEST(JobTest, CannotCompletePendingJob)
{
    Job job(
        1,
        "test.txt",
        JobOperation::CalculateHash
    );

    EXPECT_THROW(
        job.complete("result"),
        std::logic_error
    );
}

TEST(JobQueueTest, PushAndPop)
{
    JobQueue queue;

    auto job = std::make_shared<Job>(
        1,
        "test.txt",
        JobOperation::CalculateHash
    );

    EXPECT_TRUE(queue.push(job));

    EXPECT_EQ(queue.size(), 1);
    EXPECT_EQ(queue.unfinishedJobs(), 1);

    auto receivedJob = queue.pop();

    ASSERT_NE(receivedJob, nullptr);

    EXPECT_EQ(receivedJob->id(), 1);
    EXPECT_EQ(queue.size(), 0);

    // Job уже забран worker'ом, но ещё не завершён.
    EXPECT_EQ(queue.unfinishedJobs(), 1);

    queue.taskCompleted();

    EXPECT_EQ(queue.unfinishedJobs(), 0);
}

TEST(JobQueueTest, RejectsJobsAfterShutdown)
{
    JobQueue queue;

    queue.shutdown();

    auto job = std::make_shared<Job>(
        1,
        "test.txt",
        JobOperation::CalculateHash
    );

    EXPECT_FALSE(queue.push(job));

    EXPECT_EQ(queue.size(), 0);
    EXPECT_EQ(queue.unfinishedJobs(), 0);
}

TEST(JobQueueTest, RejectsNullJob)
{
    JobQueue queue;

    EXPECT_THROW(
        queue.push(nullptr),
        std::invalid_argument
    );
}