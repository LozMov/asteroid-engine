# set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fsanitize=address -g")
# set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -fsanitize=address -g")

find_package(GTest REQUIRED)
file(GLOB TEST_SOURCES CONFIGURE_DEPENDS "*_test.cpp")
foreach(TEST_SOURCE ${TEST_SOURCES})
    get_filename_component(TEST_NAME ${TEST_SOURCE} NAME_WE)
    add_executable(${TEST_NAME} ${TEST_SOURCE})
    target_link_libraries(${TEST_NAME} PRIVATE 
        asteroid_engine
        GTest::GTest
        GTest::Main
    )
    target_include_directories(${TEST_NAME} PRIVATE src)
    gtest_discover_tests(${TEST_NAME}
        WORKING_DIRECTORY ${PROJECT_SOURCE_DIR}
        # PROPERTIES VS_DEBUGGER_WORKING_DIRECTORY "${PROJECT_SOURCE_DIR}"
    )
    # set_target_properties(${TEST_NAME} PROPERTIES FOLDER tests)
    # add_test(NAME ${TEST_NAME} COMMAND ${TEST_NAME})
endforeach()