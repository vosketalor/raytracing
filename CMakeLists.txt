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

#GLEW
add_subdirectory(external/glew)

#GLM
add_subdirectory(external/glm)

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
        src/engine/scenes/SceneMicrofacets.cpp
        src/engine/scenes/SceneMicrofacets.h
        src/gui/windows/RendererWindow.cpp
        src/gui/windows/RendererWindow.h
        src/gui/windows/MenuBar.cpp
        src/gui/windows/MenuBar.h
        src/engine/ComputeRenderer.cpp
        src/engine/ComputeRenderer.h
        src/preferences/PreferenceManager.cpp
        src/preferences/PreferenceManager.h
        external/json.hpp
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
        src/engine/scenes/SceneFresnel.cpp
        src/engine/scenes/SceneFresnel.h
        src/engine/Texture.cpp
        src/engine/Texture.h
        src/engine/shapes/GPUShapeData.h
        src/engine/scenes/SceneGPU.cpp
        src/engine/scenes/SceneGPU.h
        src/engine/GPULightSource.h
        src/engine/GPUMaterial.h

)

target_link_libraries(Raytracing PRIVATE
        glfw
        glew
        glm
        OpenGL::GL)

if(USE_TBB)
    target_link_libraries(Raytracing PRIVATE TBB::tbb)
endif()

message("CMAKE_SOURCE_DIR = ${CMAKE_SOURCE_DIR}")

add_custom_command(TARGET Raytracing POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E echo "Copying resources..."
        COMMAND ${CMAKE_COMMAND} -E copy_directory
        ${CMAKE_SOURCE_DIR}/res
        $<TARGET_FILE_DIR:Raytracing>/res)


