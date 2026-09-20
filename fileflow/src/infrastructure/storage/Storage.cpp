#include "Storage.h"

#include <fstream>
#include <stdexcept>
#include <utility>

namespace fileflow::infrastructure::storage {

    Storage::Storage(
        std::filesystem::path rootDirectory
    )
        : rootDirectory_(std::move(rootDirectory))
    {
        if (rootDirectory_.empty()) {
            throw std::invalid_argument(
                "Storage root directory cannot be empty"
            );
        }

        std::error_code error;

        // Создаём всю структуру директорий.
        //
        // error_code используем вместо исключения filesystem,
        // чтобы получить контролируемую ошибку.
        std::filesystem::create_directories(
            rootDirectory_,
            error
        );

        if (error) {
            throw std::runtime_error(
                "Failed to create storage directory: " +
                error.message()
            );
        }
    }

    void Storage::save(
        const std::string& filename,
        const std::vector<std::byte>& data
    ) const
    {
        const auto path = resolvePath(filename);

        std::ofstream file(
            path,
            std::ios::binary
        );

        if (!file) {
            throw std::runtime_error(
                "Failed to open file for writing: " +
                path.string()
            );
        }

        if (!data.empty()) {
            file.write(
                reinterpret_cast<const char*>(data.data()),
                static_cast<std::streamsize>(data.size())
            );
        }

        if (!file) {
            throw std::runtime_error(
                "Failed to write file: " +
                path.string()
            );
        }
    }

    std::vector<std::byte> Storage::load(
        const std::string& filename
    ) const
    {
        const auto path = resolvePath(filename);

        std::ifstream file(
            path,
            std::ios::binary
        );

        if (!file) {
            throw std::runtime_error(
                "Failed to open file for reading: " +
                path.string()
            );
        }

        // Переходим в конец, чтобы определить размер файла.
        file.seekg(0, std::ios::end);

        const auto size = file.tellg();

        if (size < 0) {
            throw std::runtime_error(
                "Failed to determine file size: " +
                path.string()
            );
        }

        file.seekg(0, std::ios::beg);

        std::vector<std::byte> data(
            static_cast<std::size_t>(size)
        );

        if (!data.empty()) {
            file.read(
                reinterpret_cast<char*>(data.data()),
                static_cast<std::streamsize>(data.size())
            );
        }

        if (!file && !file.eof()) {
            throw std::runtime_error(
                "Failed to read file: " +
                path.string()
            );
        }

        return data;
    }

    bool Storage::exists(
        const std::string& filename
    ) const
    {
        const auto path = resolvePath(filename);

        std::error_code error;

        const bool exists =
            std::filesystem::exists(path, error);

        if (error) {
            throw std::runtime_error(
                "Failed to check file existence: " +
                error.message()
            );
        }

        return exists;
    }

    bool Storage::remove(
        const std::string& filename
    ) const
    {
        const auto path = resolvePath(filename);

        std::error_code error;

        const bool removed =
            std::filesystem::remove(path, error);

        if (error) {
            throw std::runtime_error(
                "Failed to remove file: " +
                error.message()
            );
        }

        return removed;
    }

    const std::filesystem::path&
        Storage::rootDirectory() const noexcept
    {
        return rootDirectory_;
    }

    std::filesystem::path Storage::resolvePath(
        const std::string& filename
    ) const
    {
        if (filename.empty()) {
            throw std::invalid_argument(
                "Filename cannot be empty"
            );
        }

        const std::filesystem::path filePath(filename);

        // На данном этапе запрещаем передавать путь
        // вместо имени файла.
        //
        // Например:
        // "../secret.txt"
        // "C:\\Windows\\..."
        //
        // Это важная базовая защита от path traversal.
        if (filePath.has_parent_path() ||
            filePath.is_absolute()) {

            throw std::invalid_argument(
                "Filename must not contain a directory path: " +
                filename
            );
        }

        return rootDirectory_ / filePath;
    }

} // namespace fileflow::infrastructure::storage