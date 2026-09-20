#pragma once

#include "domain/job/Job.h"

#include <cstdint>
#include <string>

namespace fileflow::application::processing {

    struct ProcessingResult {
        std::string hash;
    };

    class FileProcessor {
    public:
        [[nodiscard]]
        ProcessingResult process(
            const domain::Job& job
        ) const;

    private:
        [[nodiscard]]
        std::uint64_t calculateHash(
            const std::string& filename
        ) const;
    };

}