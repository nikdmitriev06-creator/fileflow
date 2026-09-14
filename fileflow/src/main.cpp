#include <iostream>

#include "application/Application.h"

#include "infrastructure/config/Config.h"
#include "infrastructure/http/HttpServer.h"
#include "infrastructure/logging/Logger.h"

int main()
{
    using fileflow::application::Application;
    using fileflow::infrastructure::config::loadConfig;
    using fileflow::infrastructure::http::HttpServer;
    using fileflow::infrastructure::logging::Logger;

    try {
        const auto config = loadConfig("config.json");

        Logger::initialize();

        Application application(config);

        HttpServer server(config.httpPort);

        server.run();

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