cmake_minimum_required(VERSION 3.28)
project(asteroid_engine)

option(BUILD_DEMO "Build the demo" ON)
option(USE_IMPORTED_LIBS "Use prebuilt libraries" OFF)
option(BUILD_BENCHMARK "Build benchmark" OFF)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

set(PREBUILT_LIB_DIR "${CMAKE_SOURCE_DIR}/external/lib")
set(INCLUDE_DIR "${CMAKE_SOURCE_DIR}/external/include")

if(CMAKE_BUILD_TYPE STREQUAL "Debug")
    set(CMAKE_RUNTIME_OUTPUT_DIRECTORY "${CMAKE_CURRENT_BINARY_DIR}/bin")
    set(CMAKE_LIBRARY_OUTPUT_DIRECTORY "${CMAKE_CURRENT_BINARY_DIR}/bin")
    set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY "${CMAKE_CURRENT_BINARY_DIR}/bin/lib")
else()
    set(CMAKE_RUNTIME_OUTPUT_DIRECTORY "${CMAKE_CURRENT_BINARY_DIR}/bin/Release")
    set(CMAKE_LIBRARY_OUTPUT_DIRECTORY "${CMAKE_CURRENT_BINARY_DIR}/bin/Release")
    set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY "${CMAKE_CURRENT_BINARY_DIR}/bin/Release/lib")
endif()

find_package(SDL3 CONFIG REQUIRED)
find_package(SDL3_ttf CONFIG REQUIRED)
find_package(SDL3_image CONFIG REQUIRED)
find_package(box2d CONFIG REQUIRED)
find_package(nlohmann_json CONFIG REQUIRED)
find_package(spdlog CONFIG REQUIRED)
add_library(minimp3 INTERFACE)
target_include_directories(minimp3 INTERFACE "${INCLUDE_DIR}/minimp3")

add_compile_definitions(SPDLOG_ACTIVE_LEVEL=SPDLOG_LEVEL_TRACE)

file(GLOB_RECURSE ENGINE_SOURCES CONFIGURE_DEPENDS
    "src/engine/*.cpp"
)

add_library(asteroid_engine ${ENGINE_SOURCES})

target_link_libraries(asteroid_engine PUBLIC 
    SDL3::SDL3
    SDL3_image::SDL3_image
    SDL3_ttf::SDL3_ttf
    nlohmann_json::nlohmann_json
    box2d::box2d
    spdlog::spdlog
    minimp3
)

target_include_directories(asteroid_engine PUBLIC src)

target_precompile_headers(asteroid_engine PRIVATE "src/engine/pch.hpp")

add_executable(main src/main.cpp)
target_link_libraries(main PRIVATE asteroid_engine)

if (BUILD_DEMO)
    add_subdirectory(src/demo)
endif()

# Only enable tests if this is the top-level project
if (CMAKE_PROJECT_NAME STREQUAL PROJECT_NAME) 
    include(CTest)
    if (BUILD_TESTING)
        add_subdirectory(test)
    endif()
    if (BUILD_BENCHMARK)
        add_subdirectory(benchmark)
    endif()
endif()