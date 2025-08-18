project(asteroid_demo)

file(GLOB_RECURSE DEMO_SOURCES CONFIGURE_DEPENDS
    "${CMAKE_CURRENT_SOURCE_DIR}/*.cpp"
)

list(FILTER DEMO_SOURCES EXCLUDE REGEX ".*main\\.cpp$")

add_executable(asteroid_demo ${DEMO_SOURCES})

target_link_libraries(asteroid_demo PRIVATE asteroid_engine)

target_precompile_headers(asteroid_demo REUSE_FROM asteroid_engine)