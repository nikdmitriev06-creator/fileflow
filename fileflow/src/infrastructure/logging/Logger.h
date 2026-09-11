#pragma once

#include <string_view>

namespace fileflow::infrastructure::logging {

    /*
     * @brief Центральный logger приложения.
     *
     * Остальные компоненты FileFlow не работают напрямую
     * со spdlog. Они используют этот интерфейс.
     *
     * Это уменьшает связанность и позволяет позже заменить
     * библиотеку логирования без изменения бизнес-кода.
     */
    class Logger {
    public:
        /*
         * @brief Инициализирует систему логирования.
         *
         * Должен быть вызван один раз при старте приложения.
         */
        static void initialize();

        /*
         * @brief Лог сообщения уровня INFO.
         */
        static void info(std::string_view message);

        /*
         * @brief Лог сообщения уровня WARNING.
         */
        static void warning(std::string_view message);

        /*
         * @brief Лог сообщения уровня ERROR.
         */
        static void error(std::string_view message);
    };

} // namespace fileflow::infrastructure::logging
