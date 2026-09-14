#include "HttpServer.h"

#include "application/Application.h"
#include "domain/job/Job.h"
#include "infrastructure/logging/Logger.h"

#include <httplib.h>
#include <nlohmann/json.hpp>

#include <cstdint>
#include <string>

namespace fileflow::infrastructure::http {

    namespace {

        using json = nlohmann::json;

        domain::JobOperation parseOperation(
            const std::string& operation
        )
        {
            if (operation == "calculate_hash") {
                return domain::JobOperation::CalculateHash;
            }

            if (operation == "resize") {
                return domain::JobOperation::Resize;
            }

            if (operation == "convert") {
                return domain::JobOperation::Convert;
            }

            if (operation == "compress") {
                return domain::JobOperation::Compress;
            }

            throw std::invalid_argument(
                "Unknown operation: " + operation
            );
        }

    }

    HttpServer::HttpServer(
        std::uint16_t port,
        application::Application& application
    )
        : port_(port),
        application_(application)
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

        server.Post(
            "/jobs",
            [this](
                const httplib::Request& request,
                httplib::Response& response
                ) {
                    try {
                        const auto requestJson =
                            json::parse(request.body);

                        if (!requestJson.contains("filename")) {
                            response.status = 400;

                            response.set_content(
                                R"({"error":"Missing field: filename"})",
                                "application/json"
                            );

                            return;
                        }

                        if (!requestJson.contains("operation")) {
                            response.status = 400;

                            response.set_content(
                                R"({"error":"Missing field: operation"})",
                                "application/json"
                            );

                            return;
                        }

                        const auto filename =
                            requestJson.at("filename").get<std::string>();

                        const auto operationName =
                            requestJson.at("operation").get<std::string>();

                        const auto operation =
                            parseOperation(operationName);

                        const auto jobId =
                            application_.submitJob(
                                filename,
                                operation
                            );

                        const json responseJson{
                            {"id", jobId},
                            {"status", "pending"}
                        };

                        response.status = 201;

                        response.set_content(
                            responseJson.dump(),
                            "application/json"
                        );

                        infrastructure::logging::Logger::info(
                            "HTTP POST /jobs created job " +
                            std::to_string(jobId)
                        );
                    }
                    catch (const json::exception& error) {
                        response.status = 400;

                        const json errorJson{
                            {"error", "Invalid JSON"},
                            {"message", error.what()}
                        };

                        response.set_content(
                            errorJson.dump(),
                            "application/json"
                        );
                    }
                    catch (const std::invalid_argument& error) {
                        response.status = 400;

                        const json errorJson{
                            {"error", error.what()}
                        };

                        response.set_content(
                            errorJson.dump(),
                            "application/json"
                        );
                    }
                    catch (const std::exception& error) {
                        response.status = 500;

                        const json errorJson{
                            {"error", "Internal server error"},
                            {"message", error.what()}
                        };

                        response.set_content(
                            errorJson.dump(),
                            "application/json"
                        );

                        infrastructure::logging::Logger::error(
                            "Failed to create job: " +
                            std::string(error.what())
                        );
                    }
            }
        );

        infrastructure::logging::Logger::info(
            "HTTP server listening on port " +
            std::to_string(port_)
        );

        server.listen(
            "0.0.0.0",
            port_
        );
    }

}