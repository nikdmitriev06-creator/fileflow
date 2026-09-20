#include "application/Application.h"
#include "infrastructure/config/Config.h"
#include "infrastructure/logging/Logger.h"

#include <httplib.h>
#include <nlohmann/json.hpp>

#include <gtest/gtest.h>

#include <cstdint>
#include <memory>
#include <string>
#include <thread>

using fileflow::application::Application;
using fileflow::infrastructure::config::Config;
using fileflow::infrastructure::logging::Logger;

namespace {

    using json = nlohmann::json;

    /**
     * @brief Интеграционный тест HTTP API.
     *
     * Здесь мы запускаем настоящий Application и настоящий HttpServer.
     *
     * Это отличается от unit-теста:
     * запрос проходит через HTTP -> Application -> JobQueue -> WorkerPool.
     */
    class HttpServerTest : public ::testing::Test {
    protected:
        static constexpr std::uint16_t TestPort = 18080;

        void SetUp() override
        {
            // Logger используется Application/HttpServer.
            // initialize() безопасен при повторных вызовах благодаря call_once.
            Logger::initialize();

            Config config;
            config.workerCount = 1;
            config.storageDirectory = "storage/http_tests";
            config.httpPort = TestPort;

            application =
                std::make_unique<Application>(config);

            // HTTP-сервер запускается в отдельном потоке,
            // потому что Application::run() блокируется внутри listen().
            serverThread =
                std::thread([this]() {
                application->run();
                    });

            // Ждём, пока HTTP-сервер начнёт принимать соединения.
            httplib::Client client(
                "localhost",
                TestPort
            );

            for (int attempt = 0; attempt < 50; ++attempt) {
                if (client.is_socket_open()) {
                    break;
                }

                std::this_thread::sleep_for(
                    std::chrono::milliseconds(20)
                );
            }
        }

        void TearDown() override
        {
            if (application) {
                application->stopHttpServer();
            }

            if (serverThread.joinable()) {
                serverThread.join();
            }

            application.reset();
        }

        std::unique_ptr<Application> application;
        std::thread serverThread;
    };

    TEST_F(HttpServerTest, HealthEndpoint)
    {
        httplib::Client client(
            "localhost",
            TestPort
        );

        const auto response =
            client.Get("/health");

        ASSERT_TRUE(response);

        EXPECT_EQ(
            response->status,
            200
        );

        EXPECT_EQ(
            response->get_header_value("Content-Type"),
            "application/json"
        );

        const auto responseJson =
            json::parse(response->body);

        EXPECT_EQ(
            responseJson.at("status"),
            "ok"
        );
    }

    TEST_F(HttpServerTest, CreateJob)
    {
        httplib::Client client(
            "localhost",
            TestPort
        );

        const json requestJson{
            {"filename", "test.txt"},
            {"operation", "calculate_hash"}
        };

        const auto response =
            client.Post(
                "/jobs",
                requestJson.dump(),
                "application/json"
            );

        ASSERT_TRUE(response);

        EXPECT_EQ(
            response->status,
            201
        );

        const auto responseJson =
            json::parse(response->body);

        EXPECT_TRUE(
            responseJson.contains("id")
        );

        EXPECT_EQ(
            responseJson.at("status"),
            "pending"
        );
    }

    TEST_F(HttpServerTest, GetExistingJob)
    {
        httplib::Client client(
            "localhost",
            TestPort
        );

        const json requestJson{
            {"filename", "test.txt"},
            {"operation", "calculate_hash"}
        };

        const auto createResponse =
            client.Post(
                "/jobs",
                requestJson.dump(),
                "application/json"
            );

        ASSERT_TRUE(createResponse);
        ASSERT_EQ(createResponse->status, 201);

        const auto createJson =
            json::parse(createResponse->body);

        const auto jobId =
            createJson.at("id").get<std::uint64_t>();

        const auto response =
            client.Get(
                "/jobs/" + std::to_string(jobId)
            );

        ASSERT_TRUE(response);

        EXPECT_EQ(
            response->status,
            200
        );

        const auto responseJson =
            json::parse(response->body);

        EXPECT_EQ(
            responseJson.at("id").get<std::uint64_t>(),
            jobId
        );

        EXPECT_EQ(
            responseJson.at("filename"),
            "test.txt"
        );

        EXPECT_EQ(
            responseJson.at("operation"),
            "calculate_hash"
        );

        EXPECT_TRUE(
            responseJson.contains("status")
        );
    }

    TEST_F(HttpServerTest, GetUnknownJobReturnsNotFound)
    {
        httplib::Client client(
            "localhost",
            TestPort
        );

        const auto response =
            client.Get("/jobs/999999");

        ASSERT_TRUE(response);

        EXPECT_EQ(
            response->status,
            404
        );

        const auto responseJson =
            json::parse(response->body);

        EXPECT_EQ(
            responseJson.at("error"),
            "Job not found"
        );
    }

    TEST_F(HttpServerTest, UploadFile)
    {
        httplib::Client client(
            "localhost",
            TestPort
        );

        const std::string fileContent =
            "Hello from FileFlow!";

        httplib::MultipartFormDataItems items = {
            {
                "file",
                fileContent,
                "hello.txt",
                "text/plain"
            },
            {
                "operation",
                "calculate_hash",
                "",
                "text/plain"
            }
        };

        const auto response =
            client.Post(
                "/files",
                items
            );

        ASSERT_TRUE(response);

        ASSERT_EQ(
            response->status,
            201
        );

        const auto responseJson =
            json::parse(response->body);

        ASSERT_TRUE(
            responseJson.contains("id")
        );

        EXPECT_EQ(
            responseJson.at("filename"),
            "hello.txt"
        );

        EXPECT_EQ(
            responseJson.at("status"),
            "pending"
        );

        const auto jobId =
            responseJson.at("id").get<std::uint64_t>();

        /*
         * Worker работает асинхронно, поэтому сразу после POST
         * Job ещё может находиться в pending/processing.
         *
         * Polling здесь используется только как синхронизация
         * integration test с асинхронным worker'ом.
         */
        for (int attempt = 0; attempt < 100; ++attempt) {
            const auto jobResponse =
                client.Get(
                    "/jobs/" + std::to_string(jobId)
                );

            ASSERT_TRUE(jobResponse);

            const auto jobJson =
                json::parse(jobResponse->body);

            const auto status =
                jobJson.at("status").get<std::string>();

            if (status == "completed") {
                EXPECT_TRUE(
                    jobJson.contains("result")
                );

                return;
            }

            if (status == "failed") {
                FAIL() << "Uploaded file processing failed: "
                    << jobJson.dump();
            }

            std::this_thread::sleep_for(
                std::chrono::milliseconds(20)
            );
        }

        FAIL() << "Job was not completed in time";
    }

}

