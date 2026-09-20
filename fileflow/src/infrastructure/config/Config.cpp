#include "Config.h"

#include <nlohmann/json.hpp>

#include <fstream>
#include <stdexcept>

namespace fileflow::infrastructure::config {

    Config loadConfig(const std::string& filename)
    {
        std::ifstream file(filename);

        if (!file) {
            throw std::runtime_error(
                "Failed to open config file: " + filename
            );
        }

        nlohmann::json json;

        try {
            file >> json;
        }
        catch (const nlohmann::json::parse_error& error) {
            throw std::runtime_error(
                "Invalid JSON configuration: " +
                std::string(error.what())
            );
        }

        Config config;

        // value() позволяет использовать значение по умолчанию,
        // если соответствующего поля нет в JSON.
        config.workerCount =
            json.value("worker_count", std::size_t{ 3 });

        config.logLevel =
            json.value("log_level", std::string{ "info" });

        config.storageDirectory =
            json.value(
                "storage_directory",
                std::string{ "storage" }
            );

        config.httpPort =
            json.value("http_port", std::uint16_t{ 8080 });

        // Нулевое количество worker'ов не имеет смысла:
        // задачи никогда не будут обработаны.
        if (config.workerCount == 0) {
            throw std::runtime_error(
                "worker_count must be greater than zero"
            );
        }

        return config;
    }

} // namespace fileflow::infrastructure::config