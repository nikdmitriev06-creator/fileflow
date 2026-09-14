#pragma once

#include <filesystem>
#include <string>
#include <vector>

namespace fileflow::infrastructure::storage {

    /**
     * @brief Отвечает за хранение файлов на диске.
     *
     * Storage скрывает детали работы с std::filesystem
     * от остальных компонентов приложения.
     */
    class Storage {
    public:
        /**
         * @param rootDirectory Корневая директория хранилища.
         *
         * Если директории не существует, она будет создана.
         *
         * @throws std::runtime_error если директорию невозможно создать.
         */
        explicit Storage(
            std::filesystem::path rootDirectory
        );

        /**
         * @brief Сохраняет файл.
         *
         * @param filename Имя файла внутри storage.
         * @param data Данные файла.
         *
         * @throws std::runtime_error при ошибке записи.
         */
        void save(
            const std::string& filename,
            const std::vector<std::byte>& data
        ) const;

        /**
         * @brief Загружает файл целиком в память.
         *
         * @throws std::runtime_error если файл невозможно прочитать.
         */
        [[nodiscard]]
        std::vector<std::byte> load(
            const std::string& filename
        ) const;

        /**
         * @brief Проверяет существование файла.
         */
        [[nodiscard]]
        bool exists(
            const std::string& filename
        ) const;

        /**
         * @brief Удаляет файл.
         *
         * @return true, если файл существовал и был удалён.
         */
        bool remove(
            const std::string& filename
        ) const;

        /**
         * @brief Возвращает корневую директорию storage.
         */
        [[nodiscard]]
        const std::filesystem::path& rootDirectory() const noexcept;

    private:
        /**
         * @brief Создаёт полный путь к файлу.
         *
         * Все операции с файлами проходят через этот метод.
         */
        [[nodiscard]]
        std::filesystem::path resolvePath(
            const std::string& filename
        ) const;

        std::filesystem::path rootDirectory_;
    };

} // namespace fileflow::infrastructure::storage
