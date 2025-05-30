cmake_minimum_required(VERSION 3.10)
project(Raytracing)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# GLFW
add_subdirectory(external/glfw)
include_directories(external/glfw/include)
cmake_policy(SET CMP0072 NEW)
find_package(OpenGL REQUIRED)

# ImGui
include_directories(src/engine)
include_directories(external/imgui)
include_directories(external/imgui/backends)

# TBB only on non-Windows systems
if(NOT WIN32)
    find_package(TBB REQUIRED)
    set(USE_TBB TRUE)
endif()

set(IMGUI_SOURCES
    external/imgui/imgui.cpp
    external/imgui/imgui_draw.cpp
    external/imgui/imgui_tables.cpp
    external/imgui/imgui_widgets.cpp
    external/imgui/imgui_demo.cpp
    external/imgui/backends/imgui_impl_glfw.cpp
    external/imgui/backends/imgui_impl_opengl3.cpp
    src/engine/LightSource.cpp
    src/engine/LightSource.h
    src/engine/Material.cpp
    src/engine/Material.h
        src/engine/BoundingBox.cpp
        src/engine/BoundingBox.h
)

add_executable(Raytracing
    src/InteractiveMain.cpp
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
    src/engine/shapes/Plane.cpp
    src/engine/shapes/Plane.h
    ${IMGUI_SOURCES}
)

target_link_libraries(Raytracing PRIVATE glfw OpenGL::GL)

if(USE_TBB)
    target_link_libraries(Raytracing PRIVATE TBB::tbb)
endif()
