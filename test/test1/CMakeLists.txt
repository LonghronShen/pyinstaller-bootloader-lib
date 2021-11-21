project(${project_name}-test-1 C CXX)

enable_testing()

file(GLOB_RECURSE test_src
        ${CMAKE_CURRENT_LIST_DIR}/*.h
        ${CMAKE_CURRENT_LIST_DIR}/*.c
        ${CMAKE_CURRENT_LIST_DIR}/*.cc
        ${CMAKE_CURRENT_LIST_DIR}/*.cpp
)

add_executable(${project_name}-test-1 ${test_src})

target_link_libraries(${project_name}-test-1
    PUBLIC gtest_main
    PUBLIC ${project_name}-lib
)

gtest_discover_tests(${project_name}-test-1)