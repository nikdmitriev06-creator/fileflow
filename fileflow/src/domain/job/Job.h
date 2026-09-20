#pragma once

#include <chrono>
#include <cstdint>
#include <mutex>
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

    // Снимок состояния Job.
    // Используется HTTP-потоком для безопасного чтения состояния,
    // пока worker-поток может одновременно изменять сам Job.
    struct JobSnapshot;

    class Job {
    public:
        using Id = std::uint64_t;
        using TimePoint = std::chrono::system_clock::time_point;

        Job(
            Id id,
            std::string filename,
            JobOperation operation
        );

        [[nodiscard]]
        Id id() const noexcept;

        [[nodiscard]]
        const std::string& filename() const noexcept;

        [[nodiscard]]
        JobOperation operation() const noexcept;

        [[nodiscard]]
        JobStatus status() const noexcept;

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

        // Создаёт безопасную копию текущего состояния Job.
        [[nodiscard]]
        JobSnapshot snapshot() const;

        void start();

        void complete(std::string result);

        void fail(std::string error);

    private:
        Id id_;
        std::string filename_;
        JobOperation operation_;

        JobStatus status_;

        TimePoint createdAt_;
        std::optional<TimePoint> startedAt_;
        std::optional<TimePoint> completedAt_;

        std::optional<std::string> error_;
        std::optional<std::string> result_;

        // Worker изменяет Job, а HTTP может одновременно читать его.
        // Mutex защищает состояние от data race.
        mutable std::mutex mutex_;
    };

    struct JobSnapshot {
        Job::Id id;
        std::string filename;
        JobOperation operation;
        JobStatus status;
        Job::TimePoint createdAt;
        std::optional<Job::TimePoint> startedAt;
        std::optional<Job::TimePoint> completedAt;
        std::optional<std::string> error;
        std::optional<std::string> result;
    };

}