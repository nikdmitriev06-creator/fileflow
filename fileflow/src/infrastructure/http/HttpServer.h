#pragma once

#include <cstdint>

namespace fileflow::application {
    class Application;
}

namespace fileflow::infrastructure::http {

    class HttpServer {
    public:
        HttpServer(
            std::uint16_t port,
            application::Application& application
        );

        void run();

    private:
        std::uint16_t port_;
        application::Application& application_;
    };

}