cmake_minimum_required(VERSION 3.10)
project(Raytracing)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

include_directories(src/engine)

add_executable(Raytracing
        src/ImageGen.cpp
        src/engine/Camera.cpp
        src/engine/Renderer.cpp
        src/engine/Scene.cpp
)