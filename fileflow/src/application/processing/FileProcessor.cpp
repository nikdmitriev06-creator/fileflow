#include "FileProcessor.h"

#include <fstream>
#include <iomanip>
#include <sstream>
#include <stdexcept>

namespace fileflow::application::processing {

    ProcessingResult FileProcessor::process(
        const domain::Job& job
    ) const
    {
        switch (job.operation()) {

        case domain::JobOperation::CalculateHash: {
            const auto hash = calculateHash(job.filename());

            // Преобразуем числовой hash в hexadecimal строку.
            std::ostringstream stream;

            stream << std::hex
                << std::setw(16)
                << std::setfill('0')
                << hash;

            return ProcessingResult{
                .hash = stream.str()
            };
        }

        case domain::JobOperation::Resize:
            throw std::runtime_error(
                "Resize operation is not implemented yet"
            );

        case domain::JobOperation::Convert:
            throw std::runtime_error(
                "Convert operation is not implemented yet"
            );

        case domain::JobOperation::Compress:
            throw std::runtime_error(
                "Compress operation is not implemented yet"
            );
        }

        // Это место технически недостижимо при корректном
        // значении enum, но компилятор не всегда может это доказать.
        throw std::runtime_error("Unknown job operation");
    }

    std::uint64_t FileProcessor::calculateHash(
        const std::string& filename
    ) const
    {
        // Открываем файл в бинарном режиме.
        //
        // Нам не нужно интерпретировать содержимое как текст.
        // Мы должны прочитать именно последовательность байтов.
        std::ifstream file(
            filename,
            std::ios::binary
        );

        if (!file) {
            throw std::runtime_error(
                "Failed to open file: " + filename
            );
        }

        // Начальное значение FNV-1a для 64-bit.
        std::uint64_t hash = 14695981039346656037ULL;

        // FNV prime для 64-bit.
        constexpr std::uint64_t fnvPrime = 1099511628211ULL;

        // Читаем файл небольшими блоками.
        //
        // Это важно:
        // мы не загружаем весь файл в RAM.
        constexpr std::size_t bufferSize = 8192;

        char buffer[bufferSize];

        while (file.read(buffer, sizeof(buffer)) || file.gcount() > 0) {

            // gcount() показывает, сколько байт реально было прочитано.
            const auto bytesRead = file.gcount();

            for (std::streamsize i = 0; i < bytesRead; ++i) {

                // XOR с текущим байтом.
                hash ^= static_cast<unsigned char>(buffer[i]);

                // Умножение на FNV prime.
                hash *= fnvPrime;
            }
        }

        if (file.bad()) {
            throw std::runtime_error(
                "Error while reading file: " + filename
            );
        }

        return hash;
    }

} // namespace fileflow::application::processing
