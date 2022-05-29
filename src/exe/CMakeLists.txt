project(${project_name}-exe C CXX)

file(GLOB_RECURSE src
    ${CMAKE_CURRENT_LIST_DIR}/*.h
    ${CMAKE_CURRENT_LIST_DIR}/*.hpp
    ${CMAKE_CURRENT_LIST_DIR}/*.cpp)

add_executable(${project_name}-exe ${src})

target_link_libraries(${project_name}-exe
    PRIVATE ${project_name}-lib)