cmake_minimum_required(VERSION 3.28)
project(asteroid_engine)

option(BUILD_DEMO "Build the demo" ON)
option(BUILD_TESTS "Build the tests" OFF)
option(USE_IMPORTED_LIBS "Use prebuilt libraries" OFF)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

set(PREBUILT_LIB_DIR "${CMAKE_SOURCE_DIR}/external/lib")
set(INCLUDE_DIR "${CMAKE_SOURCE_DIR}/external/include")

if(CMAKE_BUILD_TYPE STREQUAL "Debug")
    set(CMAKE_RUNTIME_OUTPUT_DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}/bin")
    set(CMAKE_LIBRARY_OUTPUT_DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}/bin")
    set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}/bin/lib")
else()
    set(CMAKE_RUNTIME_OUTPUT_DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}/bin/Release")
    set(CMAKE_LIBRARY_OUTPUT_DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}/bin/Release")
    set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}/bin/Release/lib")
endif()

find_package(SDL3 REQUIRED)
find_package(SDL3_ttf REQUIRED)
find_package(SDL3_image REQUIRED)
find_package(box2d REQUIRED)

include(FetchContent)
FetchContent_Declare(
    json
    URL https://github.com/nlohmann/json/releases/download/v3.11.3/json.tar.xz
    DOWNLOAD_EXTRACT_TIMESTAMP ON
)
FetchContent_MakeAvailable(json)

add_library(minimp3 INTERFACE)
target_include_directories(minimp3 INTERFACE "${INCLUDE_DIR}/minimp3")

add_library(spdlog STATIC IMPORTED)
set_target_properties(spdlog PROPERTIES
    IMPORTED_LOCATION "${PREBUILT_LIB_DIR}/spdlog.lib"
    INTERFACE_INCLUDE_DIRECTORIES "${INCLUDE_DIR}"
)
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
    box2d
    spdlog
    minimp3
)

target_include_directories(asteroid_engine PUBLIC src)

target_precompile_headers(asteroid_engine PRIVATE "src/engine/pch.hpp")

if (BUILD_DEMO)
    add_subdirectory(src/demo)
endif()

if(BUILD_TESTS)
    find_package(GTest REQUIRED)
    file(GLOB TEST_SOURCES "src/test/test_*.cpp" "src/test/*Test.cpp")
    # Create individual test executables
    foreach(TEST_SOURCE ${TEST_SOURCES})
        get_filename_component(TEST_NAME ${TEST_SOURCE} NAME_WE)
        add_executable(${TEST_NAME} ${TEST_SOURCE})
        target_link_libraries(${TEST_NAME} PRIVATE 
            asteroid_engine
            GTest::GTest 
            GTest::Main)
        target_include_directories(${TEST_NAME} PRIVATE src)
        add_test(NAME ${TEST_NAME} COMMAND ${TEST_NAME})
    endforeach()
    enable_testing()
    add_subdirectory(src/test)
endif()