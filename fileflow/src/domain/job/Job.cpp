#include "Job.h"

#include <stdexcept>
#include <utility>

namespace fileflow::domain {

    Job::Job(
        Id id,
        std::string filename,
        JobOperation operation
    )
        : id_(id),
        filename_(std::move(filename)),
        operation_(operation),
        status_(JobStatus::Pending),
        createdAt_(std::chrono::system_clock::now())
    {
    }

    Job::Id Job::id() const noexcept
    {
        return id_;
    }

    const std::string& Job::filename() const noexcept
    {
        return filename_;
    }

    JobOperation Job::operation() const noexcept
    {
        return operation_;
    }

    JobStatus Job::status() const noexcept
    {
        return status_;
    }

    Job::TimePoint Job::createdAt() const noexcept
    {
        return createdAt_;
    }

    const std::optional<Job::TimePoint>& Job::startedAt() const noexcept
    {
        return startedAt_;
    }

    const std::optional<Job::TimePoint>& Job::completedAt() const noexcept
    {
        return completedAt_;
    }

    const std::optional<std::string>& Job::error() const noexcept
    {
        return error_;
    }

    const std::optional<std::string>& Job::result() const noexcept
    {
        return result_;
    }

    void Job::start()
    {
        if (status_ != JobStatus::Pending) {
            throw std::logic_error(
                "Job can only be started from Pending state"
            );
        }

        status_ = JobStatus::Processing;
        startedAt_ = std::chrono::system_clock::now();
    }

    void Job::complete(std::string result)
    {
        if (status_ != JobStatus::Processing) {
            throw std::logic_error(
                "Job can only be completed from Processing state"
            );
        }

        status_ = JobStatus::Completed;
        completedAt_ = std::chrono::system_clock::now();
        result_ = std::move(result);
    }

    void Job::fail(std::string error)
    {
        if (status_ != JobStatus::Processing) {
            throw std::logic_error(
                "Job can only fail from Processing state"
            );
        }

        status_ = JobStatus::Failed;
        completedAt_ = std::chrono::system_clock::now();
        error_ = std::move(error);
    }

} // namespace fileflow::domain