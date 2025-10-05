find_package(benchmark REQUIRED)

add_executable(registry_benchmark Registry_benchmark.cpp)

target_link_libraries(registry_benchmark 
    PRIVATE 
    asteroid_engine
    benchmark::benchmark
    benchmark::benchmark_main
)
target_include_directories(registry_benchmark PRIVATE src)
