project(${project_name}-exe C CXX)

file(GLOB_RECURSE src
    ${CMAKE_CURRENT_LIST_DIR}/*.h
    ${CMAKE_CURRENT_LIST_DIR}/*.c
    ${CMAKE_CURRENT_LIST_DIR}/*.cc
    ${CMAKE_CURRENT_LIST_DIR}/*.cpp
)

add_executable(${project_name}-exe ${src})

target_link_libraries(${project_name}-exe
    PUBLIC ${project_name}-lib
)
