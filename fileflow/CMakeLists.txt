cmake_minimum_required(VERSION 3.20)

project(FileFlow
    VERSION 0.1.0
    DESCRIPTION "Asynchronous file processing service"
    LANGUAGES CXX
)

include(FetchContent)

FetchContent_Declare(
    spdlog
    GIT_REPOSITORY https://github.com/gabime/spdlog.git
    GIT_TAG v1.15.3
)

FetchContent_MakeAvailable(spdlog)

FetchContent_Declare(
    nlohmann_json
    GIT_REPOSITORY https://github.com/nlohmann/json.git
    GIT_TAG v3.12.0
)

FetchContent_Declare(
    googletest
    GIT_REPOSITORY https://github.com/google/googletest.git
    GIT_TAG v1.17.0
)

FetchContent_MakeAvailable(googletest)

FetchContent_MakeAvailable(nlohmann_json)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# ============================================================
# Main application
# ============================================================

add_executable(fileflow
    src/main.cpp

    src/application/Application.cpp
    src/application/processing/FileProcessor.cpp

    src/domain/job/Job.cpp
    src/domain/job/JobQueue.cpp
    src/domain/job/WorkerPool.cpp

    src/infrastructure/logging/Logger.cpp
    src/infrastructure/config/Config.cpp
    src/infrastructure/storage/Storage.cpp
)

configure_file(
    ${CMAKE_CURRENT_SOURCE_DIR}/config.json
    ${CMAKE_CURRENT_BINARY_DIR}/config.json
    COPYONLY
)


target_include_directories(fileflow PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/src
)

target_link_libraries(fileflow PRIVATE
    spdlog::spdlog
    nlohmann_json::nlohmann_json
)

target_compile_features(fileflow PRIVATE cxx_std_20)

if (MSVC)
    target_compile_options(fileflow PRIVATE
        /W4
        /permissive-
        /utf-8
    )
else()
    target_compile_options(fileflow PRIVATE
        -Wall
        -Wextra
        -Wpedantic
    )
endif()


# ============================================================
# Tests
# ============================================================

enable_testing()

add_executable(fileflow_tests
    tests/JobTests.cpp
    tests/StorageTests.cpp
    tests/FileProcessorTests.cpp

    src/domain/job/Job.cpp
    src/domain/job/JobQueue.cpp

    src/infrastructure/storage/Storage.cpp

    src/application/processing/FileProcessor.cpp
)

target_include_directories(fileflow_tests PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/src
)

target_compile_features(fileflow_tests PRIVATE
    cxx_std_20
)

target_link_libraries(fileflow_tests PRIVATE
    GTest::gtest_main
)

if (MSVC)
    target_compile_options(fileflow_tests PRIVATE
        /W4
        /permissive-
        /utf-8
    )
else()
    target_compile_options(fileflow_tests PRIVATE
        -Wall
        -Wextra
        -Wpedantic
    )
endif()

include(GoogleTest)

gtest_discover_tests(fileflow_tests)