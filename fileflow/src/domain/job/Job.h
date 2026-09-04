#pragma once

#include <cstdint>
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

        void start() noexcept;
        void complete() noexcept;
        void fail() noexcept;

    private:
        Id id_;
        std::string filename_;
        JobOperation operation_;
        JobStatus status_;
    };

} // namespace fileflow::domain

