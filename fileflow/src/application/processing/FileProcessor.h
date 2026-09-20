#pragma once

#include "domain/job/Job.h"

#include <cstdint>
#include <string>

namespace fileflow::application::processing {

    /**
     * @brief Результат обработки файла.
     *
     * Пока процессор возвращает только hash.
     * Позже сюда можно добавить:
     * - outputPath;
     * - fileSize;
     * - processingTime;
     * - metadata;
     * и т.д.
     */
    struct ProcessingResult {
        std::string hash;
    };

    /**
     * @brief Выполняет операции над файлами.
     *
     * FileProcessor не знает о:
     * - HTTP;
     * - JobQueue;
     * - WorkerPool;
     * - PostgreSQL.
     *
     * Он получает Job и выполняет непосредственно
     * операцию над файлом.
     */
    class FileProcessor {
    public:
        /**
         * @brief Обрабатывает файл согласно типу JobOperation.
         *
         * @throws std::runtime_error если файл невозможно открыть
         *         или произошла ошибка обработки.
         */
        [[nodiscard]]
        ProcessingResult process(
            const domain::Job& job
        ) const;

    private:
        /**
         * @brief Вычисляет FNV-1a hash файла.
         */
        [[nodiscard]]
        std::uint64_t calculateHash(
            const std::string& filename
        ) const;
    };

} // namespace fileflow::application::processing
