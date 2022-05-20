project(game)

cmake_minimum_required(VERSION 3.15)

add_subdirectory(box2d)

include_directories(
    ./
    ./imgui
    ./include
)

link_libraries(box2d sfml-graphics sfml-system sfml-window sfml-audio sfml-network GL GLU)

file(GLOB_RECURSE SRC
    main.cpp
    imgui/*.cpp
)

add_executable(${PROJECT_NAME} ${SRC})
