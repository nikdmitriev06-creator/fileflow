#pragma once

#include <cstdint>
#include <optional>
#include <string>

namespace fileflow::domain {

    enum class JobStatus {
        Pending,
        Processing,
        Completed,
        Failed
    };

    enum class JobOperation {
        CalculateHash,
        Resize,
        Convert,
        Compress
    };

    /**
     * @brief Описывает одну задачу FileFlow.
     *
     * Job содержит только состояние и данные задачи.
     * Он не знает о WorkerPool, очереди или обработке файлов.
     */
    class Job {
    public:
        using Id = std::uint64_t;

        Job(
            Id id,
            std::string filename,
            JobOperation operation
        );

        [[nodiscard]] Id id() const noexcept;
        [[nodiscard]] const std::string& filename() const noexcept;
        [[nodiscard]] JobOperation operation() const noexcept;
        [[nodiscard]] JobStatus status() const noexcept;

        /**
         * @brief Переводит задачу из Pending в Processing.
         *
         * @throws std::logic_error если задача уже была запущена.
         */
        void start();

        /**
         * @brief Переводит задачу из Processing в Completed.
         *
         * @throws std::logic_error если задача не находится в Processing.
         */
        void complete();

        /**
         * @brief Переводит задачу из Processing в Failed.
         *
         * @param error Описание ошибки.
         */
        void fail(std::string error);

        /**
         * @brief Возвращает описание ошибки.
         *
         * Для успешно выполненной задачи значение отсутствует.
         */
        [[nodiscard]]
        const std::optional<std::string>& error() const noexcept;

    private:
        Id id_;
        std::string filename_;
        JobOperation operation_;

        JobStatus status_;

        // Заполняется только если задача завершилась с ошибкой.
        std::optional<std::string> error_;
    };

} // namespace fileflow::domain