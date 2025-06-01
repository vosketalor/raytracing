cmake_minimum_required(VERSION 3.10)
set(CMAKE_C_COMPILER_WORKS TRUE CACHE BOOL "Force C compiler check" FORCE)
set(CMAKE_CXX_COMPILER_WORKS TRUE CACHE BOOL "Force CXX compiler check" FORCE)
project(Raytracing)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
cmake_policy(SET CMP0072 NEW)

# =========================
# Detect compiler
# =========================
set(IS_NVHPC OFF)
if(CMAKE_CXX_COMPILER_ID STREQUAL "NVHPC")
    message(STATUS "Using NVIDIA HPC compiler with OpenACC support")
    set(IS_NVHPC ON)
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -acc -gpu=cc70,cuda11.8 -Minfo=accel")
    set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -acc -gpu=cc70,cuda11.8 -Minfo=accel")
elseif(CMAKE_CXX_COMPILER_ID STREQUAL "GNU")
    message(STATUS "Using GCC with OpenACC support")
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fopenacc")
    set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -fopenacc")
endif()

# =========================
# GLFW: special handling if NVHPC
# =========================
if(IS_NVHPC)
    set(GLFW_BINARY_DIR ${CMAKE_SOURCE_DIR}/external/glfw-bin)

    if(NOT EXISTS "${GLFW_BINARY_DIR}/libglfw3.a")
        message(STATUS "GLFW not yet built with GCC — building now...")

        execute_process(COMMAND ${CMAKE_COMMAND}
                -DCMAKE_CXX_COMPILER=g++
                -DCMAKE_C_COMPILER=gcc
                -DGLFW_BUILD_EXAMPLES=OFF
                -DGLFW_BUILD_TESTS=OFF
                -DGLFW_BUILD_DOCS=OFF
                -DCMAKE_BUILD_TYPE=Release
                -S ${CMAKE_SOURCE_DIR}/external/glfw
                -B ${GLFW_BINARY_DIR})

        execute_process(COMMAND ${CMAKE_COMMAND}
                --build ${GLFW_BINARY_DIR})
    endif()

    include_directories(${CMAKE_SOURCE_DIR}/external/glfw/include)
    link_directories(${GLFW_BINARY_DIR}/src)
    set(GLFW_LIB glfw3)
else()
    set(GLFW_BUILD_EXAMPLES OFF CACHE BOOL "")
    set(GLFW_BUILD_TESTS OFF CACHE BOOL "")
    set(GLFW_BUILD_DOCS OFF CACHE BOOL "")
    add_subdirectory(external/glfw)
    include_directories(external/glfw/include)
    set(GLFW_LIB glfw)
endif()

find_package(OpenGL REQUIRED)

# =========================
# ImGui
# =========================
include_directories(
        src/engine
        external/imgui
        external/imgui/backends
)

# =========================
# TBB (Unix only)
# =========================
if(NOT WIN32)
    find_package(TBB REQUIRED)
    set(USE_TBB TRUE)
endif()

# =========================
# Sources
# =========================
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
        src/engine/acceleration/BoundingBox.cpp
        src/engine/acceleration/BoundingBox.h
        src/engine/acceleration/BVHNode.cpp
        src/engine/acceleration/BVHNode.h
        src/engine/shapes/Triangle.cpp
        src/engine/shapes/Triangle.h
        src/engine/shapes/OBJ.cpp
        src/engine/shapes/OBJ.h
        src/gui/Application.cpp
        src/gui/Application.h
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
        src/gui/Application.cpp
        src/gui/Application.h
        src/gui/UIManager.cpp
        src/gui/UIManager.h
        src/gui/windows/MainViewport.cpp
        src/gui/windows/MainViewport.h
        src/gui/windows/ResolutionWindow.cpp
        src/gui/windows/ResolutionWindow.h
        src/gui/windows/StatisticsWindow.cpp
        src/gui/windows/StatisticsWindow.h
        src/gui/windows/UIWindow.cpp
        src/gui/windows/UIWindow.h
        src/gui/Window.cpp
        src/gui/Window.h
        src/gui/InputManager.cpp
        src/gui/InputManager.h
        src/gui/WindowConfig.h
        ${IMGUI_SOURCES}
)

target_link_libraries(Raytracing PRIVATE ${GLFW_LIB} OpenGL::GL)

if(USE_TBB)
    target_link_libraries(Raytracing PRIVATE TBB::tbb)
endif()

if(IS_NVHPC)
    target_link_libraries(Raytracing PRIVATE accel)
endif()

add_custom_command(TARGET Raytracing POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E echo "Copying resources..."
        COMMAND ${CMAKE_COMMAND} -E copy_directory
        ${CMAKE_SOURCE_DIR}/res
        $<TARGET_FILE_DIR:Raytracing>/res
)
