#include "application/processing/FileProcessor.h"

#include "domain/job/Job.h"

#include <gtest/gtest.h>

#include <filesystem>
#include <fstream>
#include <stdexcept>

using fileflow::application::processing::FileProcessor;
using fileflow::domain::Job;
using fileflow::domain::JobOperation;

namespace {

    class FileProcessorTest : public ::testing::Test {
    protected:
        void SetUp() override
        {
            testDirectory =
                std::filesystem::temp_directory_path() /
                "fileflow_processor_tests";

            std::filesystem::remove_all(testDirectory);

            std::filesystem::create_directories(
                testDirectory
            );
        }

        void TearDown() override
        {
            std::filesystem::remove_all(testDirectory);
        }

        std::filesystem::path testDirectory;
    };

} // namespace

TEST_F(FileProcessorTest, CalculatesHash)
{
    const auto filePath =
        testDirectory / "test.txt";

    {
        std::ofstream file(
            filePath,
            std::ios::binary
        );

        ASSERT_TRUE(file);

        file << "Hello FileFlow";
    }

    FileProcessor processor;

    const Job job(
        1,
        filePath.string(),
        JobOperation::CalculateHash
    );

    const auto result =
        processor.process(job);

    EXPECT_FALSE(result.hash.empty());
    EXPECT_EQ(result.hash.length(), 16);

    // ѕровер€ем детерминированность алгоритма:
    // одинаковое содержимое должно давать одинаковый hash.
    const auto secondResult =
        processor.process(job);

    EXPECT_EQ(
        result.hash,
        secondResult.hash
    );
}

TEST_F(FileProcessorTest, DifferentContentsProduceDifferentHashes)
{
    const auto firstPath =
        testDirectory / "first.txt";

    const auto secondPath =
        testDirectory / "second.txt";

    {
        std::ofstream file(
            firstPath,
            std::ios::binary
        );

        ASSERT_TRUE(file);

        file << "Hello";
    }

    {
        std::ofstream file(
            secondPath,
            std::ios::binary
        );

        ASSERT_TRUE(file);

        file << "World";
    }

    FileProcessor processor;

    const Job firstJob(
        1,
        firstPath.string(),
        JobOperation::CalculateHash
    );

    const Job secondJob(
        2,
        secondPath.string(),
        JobOperation::CalculateHash
    );

    const auto firstResult =
        processor.process(firstJob);

    const auto secondResult =
        processor.process(secondJob);

    EXPECT_NE(
        firstResult.hash,
        secondResult.hash
    );
}

TEST_F(FileProcessorTest, ThrowsForMissingFile)
{
    FileProcessor processor;

    const Job job(
        1,
        (testDirectory / "missing.txt").string(),
        JobOperation::CalculateHash
    );

    EXPECT_THROW(
        processor.process(job),
        std::runtime_error
    );
}

TEST_F(FileProcessorTest, ThrowsForUnsupportedOperation)
{
    FileProcessor processor;

    const Job job(
        1,
        "test.txt",
        JobOperation::Resize
    );

    EXPECT_THROW(
        processor.process(job),
        std::runtime_error
    );
}