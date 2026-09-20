#include <iostream>

#include "application/Application.h"

#include "infrastructure/config/Config.h"
#include "infrastructure/logging/Logger.h"

int main()
{
    using fileflow::application::Application;
    using fileflow::infrastructure::config::loadConfig;
    using fileflow::infrastructure::logging::Logger;

    try {
        const auto config = loadConfig("config.json");

        Logger::initialize();

        Application application(config);

        application.run();

        return 0;
    }
    catch (const std::exception& error) {
        std::cerr
            << "Fatal error: "
            << error.what()
            << '\n';

        return 1;
    }
}