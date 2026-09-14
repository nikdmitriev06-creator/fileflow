#include "HttpServer.h"

#include "infrastructure/logging/Logger.h"

#include <httplib.h>

#include <string>

namespace fileflow::infrastructure::http {

    HttpServer::HttpServer(std::uint16_t port)
        : port_(port)
    {
    }

    void HttpServer::run()
    {
        httplib::Server server;

        server.Get(
            "/health",
            [](const httplib::Request&, httplib::Response& response) {
                response.set_content(
                    R"({"status":"ok"})",
                    "application/json"
                );
            }
        );

        infrastructure::logging::Logger::info(
            "HTTP server listening on port " +
            std::to_string(port_)
        );

        server.listen("0.0.0.0", port_);
    }

}