#include "HttpServer.h"

#include "application/Application.h"
#include "domain/job/Job.h"
#include "infrastructure/logging/Logger.h"

#include <httplib.h>
#include <nlohmann/json.hpp>

#include <cstdint>
#include <memory>
#include <stdexcept>
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

        const char* statusToString(
            domain::JobStatus status
        )
        {
            switch (status) {
            case domain::JobStatus::Pending:
                return "pending";

            case domain::JobStatus::Processing:
                return "processing";

            case domain::JobStatus::Completed:
                return "completed";

            case domain::JobStatus::Failed:
                return "failed";
            }

            return "unknown";
        }

        const char* operationToString(
            domain::JobOperation operation
        )
        {
            switch (operation) {
            case domain::JobOperation::CalculateHash:
                return "calculate_hash";

            case domain::JobOperation::Resize:
                return "resize";

            case domain::JobOperation::Convert:
                return "convert";

            case domain::JobOperation::Compress:
                return "compress";
            }

            return "unknown";
        }

    } // namespace

    HttpServer::HttpServer(
        std::uint16_t port,
        application::Application& application
    )
        : port_(port),
        application_(application),
        server_(std::make_unique<httplib::Server>())
    {
    }

    HttpServer::~HttpServer()
    {
        stop();
    }

    void HttpServer::run()
    {
        server_->Get(
            "/health",
            [](const httplib::Request&, httplib::Response& response) {
                response.set_content(
                    R"({"status":"ok"})",
                    "application/json"
                );
            }
        );

        server_->Get(
            R"(/jobs/(\d+))",
            [this](
                const httplib::Request& request,
                httplib::Response& response
                ) {
                    try {
                        const auto jobId =
                            static_cast<domain::Job::Id>(
                                std::stoull(request.matches[1].str())
                                );

                        const auto job =
                            application_.findJob(jobId);

                        if (!job) {
                            response.status = 404;

                            const json errorJson{
                                {"error", "Job not found"}
                            };

                            response.set_content(
                                errorJson.dump(),
                                "application/json"
                            );

                            return;
                        }

                        // snapshot() создаёт независимую копию состояния Job.
                        // Поэтому worker может продолжать изменять Job,
                        // пока HTTP-поток формирует ответ.
                        const auto snapshot = job->snapshot();

                        json responseJson{
                            {"id", snapshot.id},
                            {"filename", snapshot.filename},
                            {"operation", operationToString(snapshot.operation)},
                            {"status", statusToString(snapshot.status)}
                        };

                        if (snapshot.result) {
                            responseJson["result"] = *snapshot.result;
                        }

                        if (snapshot.error) {
                            responseJson["error"] = *snapshot.error;
                        }

                        response.set_content(
                            responseJson.dump(),
                            "application/json"
                        );
                    }
                    catch (const std::exception& error) {
                        response.status = 400;

                        const json errorJson{
                            {"error", error.what()}
                        };

                        response.set_content(
                            errorJson.dump(),
                            "application/json"
                        );
                    }
            }
        );

        server_->Post(
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

        server_->Post(
            "/files",
            [this](const httplib::Request& request, httplib::Response& response) {
                try {
                    if (!request.is_multipart_form_data()) {
                        response.status = 400;

                        const json errorJson{
                            {"error", "Expected multipart/form-data"}
                        };

                        response.set_content(
                            errorJson.dump(),
                            "application/json"
                        );

                        return;
                    }

                    if (!request.has_file("file")) {
                        response.status = 400;

                        const json errorJson{
                            {"error", "Missing file field"}
                        };

                        response.set_content(
                            errorJson.dump(),
                            "application/json"
                        );

                        return;
                    }

                    if (!request.has_file("operation")) {
                        response.status = 400;

                        const json errorJson{
                            {"error", "Missing operation"}
                        };

                        response.set_content(
                            errorJson.dump(),
                            "application/json"
                        );

                        return;
                    }

                    const auto& file =
                        request.get_file_value("file");

                    if (file.filename.empty()) {
                        response.status = 400;

                        const json errorJson{
                            {"error", "Filename is empty"}
                        };

                        response.set_content(
                            errorJson.dump(),
                            "application/json"
                        );

                        return;
                    }

                    const auto operationField =
                        request.get_file_value("operation");

                    const auto operation =
                        parseOperation(operationField.content);

                    /*
                     * HttpServer не знает, как именно организовано
                     * хранение файла и создание Job.
                     *
                     * Он только извлекает данные HTTP-запроса
                     * и передаёт их Application.
                     */
                    const auto jobId =
                        application_.uploadFile(
                            file.filename,
                            file.content,
                            operation
                        );

                    const json responseJson{
                        {"id", jobId},
                        {"filename", file.filename},
                        {"status", "pending"}
                    };

                    response.status = 201;

                    response.set_content(
                        responseJson.dump(),
                        "application/json"
                    );

                    infrastructure::logging::Logger::info(
                        "HTTP POST /files created job " +
                        std::to_string(jobId)
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
                        "Failed to upload file: " +
                        std::string(error.what())
                    );
                }
            }
        );
        infrastructure::logging::Logger::info(
            "HTTP server listening on port " +
            std::to_string(port_)
        );

        server_->listen(
            "0.0.0.0",
            port_
        );
    }

    void HttpServer::stop()
    {
        if (server_) {
            server_->stop();
        }
    }

}