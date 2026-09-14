#include "infrastructure/storage/Storage.h"

#include <gtest/gtest.h>

#include <cstddef>
#include <filesystem>
#include <vector>

using fileflow::infrastructure::storage::Storage;

namespace {

    class StorageTest : public ::testing::Test {
    protected:
        void SetUp() override
        {
            testDirectory =
                std::filesystem::temp_directory_path() /
                "fileflow_storage_tests";

            std::filesystem::remove_all(testDirectory);
        }

        void TearDown() override
        {
            std::filesystem::remove_all(testDirectory);
        }

        std::filesystem::path testDirectory;
    };

} // namespace

TEST_F(StorageTest, CreatesRootDirectory)
{
    Storage storage(testDirectory);

    EXPECT_TRUE(
        std::filesystem::exists(testDirectory)
    );

    EXPECT_TRUE(
        std::filesystem::is_directory(testDirectory)
    );
}

TEST_F(StorageTest, SavesAndLoadsData)
{
    Storage storage(testDirectory);

    const std::vector<std::byte> data{
        std::byte{'H'},
        std::byte{'e'},
        std::byte{'l'},
        std::byte{'l'},
        std::byte{'o'}
    };

    storage.save("test.bin", data);

    EXPECT_TRUE(
        storage.exists("test.bin")
    );

    const auto loaded =
        storage.load("test.bin");

    EXPECT_EQ(loaded, data);
}

TEST_F(StorageTest, RemovesFile)
{
    Storage storage(testDirectory);

    const std::vector<std::byte> data{
        std::byte{'T'},
        std::byte{'e'},
        std::byte{'s'},
        std::byte{'t'}
    };

    storage.save("test.bin", data);

    ASSERT_TRUE(
        storage.exists("test.bin")
    );

    EXPECT_TRUE(
        storage.remove("test.bin")
    );

    EXPECT_FALSE(
        storage.exists("test.bin")
    );

    // Удаление уже отсутствующего файла
    // должно просто вернуть false.
    EXPECT_FALSE(
        storage.remove("test.bin")
    );
}

TEST_F(StorageTest, RejectsParentPath)
{
    Storage storage(testDirectory);

    const std::vector<std::byte> data{
        std::byte{'X'}
    };

    EXPECT_THROW(
        storage.save("../outside.bin", data),
        std::invalid_argument
    );
}

TEST_F(StorageTest, RejectsAbsolutePath)
{
    Storage storage(testDirectory);

    const std::vector<std::byte> data{
        std::byte{'X'}
    };

    const auto absolutePath =
        std::filesystem::temp_directory_path() /
        "outside.bin";

    EXPECT_THROW(
        storage.save(
            absolutePath.string(),
            data
        ),
        std::invalid_argument
    );
}