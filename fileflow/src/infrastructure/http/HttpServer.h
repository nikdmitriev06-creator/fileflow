#pragma once

#include <cstdint>

namespace fileflow::infrastructure::http {

    class HttpServer {
    public:
        explicit HttpServer(std::uint16_t port);

        void run();

    private:
        std::uint16_t port_;
    };

}