cmake_minimum_required(VERSION 3.21)
project(TinyEditor LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(Qt6 REQUIRED COMPONENTS Widgets OpenGL OpenGLWidgets)

qt_standard_project_setup()

qt_add_executable(TinyEditor
    src/main.cpp
    src/EditorMainWindow.h
    src/EditorMainWindow.cpp
    src/ViewportWidget.h
    src/ViewportWidget.cpp
    src/Scene.h
    src/Scene.cpp
    src/Entity.h
    src/Mesh.h
    src/Renderer.h
    src/Renderer.cpp
    src/Commands.h
    src/Commands.cpp
    src/SceneSerializer.h
    src/SceneSerializer.cpp
)

target_link_libraries(TinyEditor PRIVATE
    Qt6::Widgets
    Qt6::OpenGL
    Qt6::OpenGLWidgets
)

target_compile_definitions(TinyEditor PRIVATE
    SHADER_DIR="${CMAKE_SOURCE_DIR}/shaders"
)
