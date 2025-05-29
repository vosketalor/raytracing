cmake_minimum_required(VERSION 3.10)
project(Raytracing)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

include_directories(src/engine)

add_executable(Raytracing
        src/ImageGen.cpp
        src/engine/Camera.cpp
        src/engine/Renderer.cpp
        src/engine/scenes/Scene.cpp
        src/engine/Vector.cpp
        src/engine/scenes/Scene1.cpp
        src/engine/shapes/Shape.cpp
        src/engine/shapes/Shape.h
        src/engine/shapes/Sphere.cpp
        src/engine/shapes/Sphere.h
        src/engine/Intersection.cpp
        src/engine/Intersection.h
)