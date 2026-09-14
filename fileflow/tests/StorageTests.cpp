#include "infrastructure/storage/Storage.h"

#include <cassert>
#include <cstddef>
#include <filesystem>
#include <string>
#include <vector>

using fileflow::infrastructure::storage::Storage;

namespace {

    std::filesystem::path createTestDirectory()
    {
        const auto directory =
            std::filesystem::temp_directory_path() /
            "fileflow_storage_tests";

        std::filesystem::remove_all(directory);

        return directory;
    }

    void testStorageCreatesDirectory()
    {
        const auto directory = createTestDirectory();

        {
            Storage storage(directory);

            assert(std::filesystem::exists(directory));
            assert(std::filesystem::is_directory(directory));
        }

        std::filesystem::remove_all(directory);
    }

    void testSaveAndLoad()
    {
        const auto directory = createTestDirectory();

        {
            Storage storage(directory);

            const std::vector<std::byte> data{
                std::byte{'H'},
                std::byte{'e'},
                std::byte{'l'},
                std::byte{'l'},
                std::byte{'o'}
            };

            storage.save("test.bin", data);

            assert(storage.exists("test.bin"));

            const auto loaded = storage.load("test.bin");

            assert(loaded == data);
        }

        std::filesystem::remove_all(directory);
    }

    void testRemove()
    {
        const auto directory = createTestDirectory();

        {
            Storage storage(directory);

            const std::vector<std::byte> data{
                std::byte{'T'},
                std::byte{'e'},
                std::byte{'s'},
                std::byte{'t'}
            };

            storage.save("test.bin", data);

            assert(storage.exists("test.bin"));

            const bool removed =
                storage.remove("test.bin");

            assert(removed);
            assert(!storage.exists("test.bin"));

            // Повторное удаление несуществующего файла
            // не является ошибкой.
            assert(!storage.remove("test.bin"));
        }

        std::filesystem::remove_all(directory);
    }

    void testRejectsInvalidFilename()
    {
        const auto directory = createTestDirectory();

        {
            Storage storage(directory);

            const std::vector<std::byte> data{
                std::byte{'X'}
            };

            bool exceptionThrown = false;

            try {
                storage.save("../outside.bin", data);
            }
            catch (const std::invalid_argument&) {
                exceptionThrown = true;
            }

            assert(exceptionThrown);
        }

        std::filesystem::remove_all(directory);
    }

} // namespace

int main()
{
    testStorageCreatesDirectory();
    testSaveAndLoad();
    testRemove();
    testRejectsInvalidFilename();

    return 0;
}