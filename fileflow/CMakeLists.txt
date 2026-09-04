cmake_minimum_required(VERSION 3.20)

project(FileFlow
    VERSION 0.1.0
    DESCRIPTION "Asynchronous file processing service"
    LANGUAGES CXX
)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

add_executable(fileflow
    src/main.cpp
    src/domain/job/Job.cpp
    src/domain/job/JobQueue.cpp
    src/domain/job/WorkerPool.cpp
)

target_compile_features(fileflow PRIVATE cxx_std_20)

if (MSVC)
    target_compile_options(fileflow PRIVATE
        /W4
        /permissive-
    )
else()
    target_compile_options(fileflow PRIVATE
        -Wall
        -Wextra
        -Wpedantic
    )
endif()