#pragma once

#include <chrono>
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
     * Job содержит состояние и данные задачи.
     * Он не знает о WorkerPool, JobQueue или FileProcessor.
     */
    class Job {
    public:
        using Id = std::uint64_t;

        // Используем системное время, потому что timestamps
        // потенциально будут сохраняться в PostgreSQL.
        using TimePoint = std::chrono::system_clock::time_point;

        Job(
            Id id,
            std::string filename,
            JobOperation operation
        );

        [[nodiscard]] Id id() const noexcept;
        [[nodiscard]] const std::string& filename() const noexcept;
        [[nodiscard]] JobOperation operation() const noexcept;
        [[nodiscard]] JobStatus status() const noexcept;

        [[nodiscard]]
        TimePoint createdAt() const noexcept;

        [[nodiscard]]
        const std::optional<TimePoint>& startedAt() const noexcept;

        [[nodiscard]]
        const std::optional<TimePoint>& completedAt() const noexcept;

        [[nodiscard]]
        const std::optional<std::string>& error() const noexcept;

        [[nodiscard]]
        const std::optional<std::string>& result() const noexcept;

        /**
         * @brief Переводит задачу Pending -> Processing.
         */
        void start();

        /**
         * @brief Переводит задачу Processing -> Completed.
         *
         * @param result Результат обработки.
         */
        void complete(std::string result);

        /**
         * @brief Переводит задачу Processing -> Failed.
         *
         * @param error Описание ошибки.
         */
        void fail(std::string error);

    private:
        Id id_;
        std::string filename_;
        JobOperation operation_;

        JobStatus status_;

        // Время создания задачи устанавливается в конструкторе.
        TimePoint createdAt_;

        // Эти значения появляются только после соответствующих переходов.
        std::optional<TimePoint> startedAt_;
        std::optional<TimePoint> completedAt_;

        std::optional<std::string> error_;

        // Например:
        // hash файла, путь к результату resize и т.д.
        std::optional<std::string> result_;
    };

} // namespace fileflow::domain