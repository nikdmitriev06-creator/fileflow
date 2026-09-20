#pragma once

#include <cstdint>
#include <memory>

namespace httplib {
    class Server;
}

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

        ~HttpServer();

        HttpServer(const HttpServer&) = delete;
        HttpServer& operator=(const HttpServer&) = delete;

        // Запускает HTTP-сервер и блокируется до его остановки.
        void run();

        // Останавливает HTTP-сервер.
        // Безопасно вызывать, если сервер уже остановлен.
        void stop();

    private:
        std::uint16_t port_;

        // Application принадлежит не HttpServer.
        // HttpServer только использует его для создания и поиска Job.
        application::Application& application_;

        // Сервер теперь живёт столько же, сколько HttpServer.
        //
        // unique_ptr используется потому, что httplib::Server — тяжёлый
        // implementation type, а в header нам достаточно forward declaration.
        std::unique_ptr<httplib::Server> server_;
    };

}