#pragma once

#include <cstddef>
#include <string>

namespace fileflow::infrastructure::config {

    /**
     * @brief Конфигурация приложения FileFlow.
     *
     * Здесь находятся только настройки, необходимые
     * для запуска приложения.
     *
     * Позже сюда можно добавить:
     * - HTTP port;
     * - PostgreSQL connection string;
     * - maximum file size;
     * - worker queue size;
     * - log file path.
     */
    struct Config {
        std::size_t workerCount{ 3 };

        std::string logLevel{ "info" };

        std::string storageDirectory{ "storage" };

        std::uint16_t httpPort{ 8080 };
    };

    /**
     * @brief Загружает конфигурацию из JSON-файла.
     *
     * @throws std::runtime_error если файл невозможно открыть
     *         или конфигурация имеет неправильный формат.
     */
    [[nodiscard]]
    Config loadConfig(const std::string& filename);

} // namespace fileflow::infrastructure::config