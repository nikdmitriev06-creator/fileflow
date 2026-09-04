#include "Job.h"

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
        status_(JobStatus::Pending)
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

    void Job::start() noexcept
    {
        status_ = JobStatus::Processing;
    }

    void Job::complete() noexcept
    {
        status_ = JobStatus::Completed;
    }

    void Job::fail() noexcept
    {
        status_ = JobStatus::Failed;
    }

} // namespace fileflow::domain