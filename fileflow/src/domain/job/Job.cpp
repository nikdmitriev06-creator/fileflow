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

    void Job::start()
    {
        if (status_ != JobStatus::Pending) {
            throw std::logic_error(
                "Job can only be started from Pending state"
            );
        }

        status_ = JobStatus::Processing;
    }

    void Job::complete()
    {
        if (status_ != JobStatus::Processing) {
            throw std::logic_error(
                "Job can only be completed from Processing state"
            );
        }

        status_ = JobStatus::Completed;
    }

    void Job::fail(std::string error)
    {
        if (status_ != JobStatus::Processing) {
            throw std::logic_error(
                "Job can only fail from Processing state"
            );
        }

        status_ = JobStatus::Failed;
        error_ = std::move(error);
    }

    const std::optional<std::string>& Job::error() const noexcept
    {
        return error_;
    }

} // namespace fileflow::domain