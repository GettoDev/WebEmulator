# source files to compile graphic library glad
add_library(glad STATIC
    glad/src/glad.c)

# source files to compile GUI library imgui
add_library(imgui STATIC
    imgui/imgui_demo.cpp
    imgui/imgui_draw.cpp
    imgui/imgui_impl_opengl2.cpp
    imgui/imgui_impl_opengl3.cpp
    imgui/imgui_impl_sdl.cpp
    imgui/imgui_tables.cpp
    imgui/imgui_widgets.cpp
    imgui/imgui.cpp)

# select a specific GL loader or it will attempt to use GLEW
target_compile_definitions(imgui PUBLIC IMGUI_IMPL_OPENGL_LOADER_GLAD)
