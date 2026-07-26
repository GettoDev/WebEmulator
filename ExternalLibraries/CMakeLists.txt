# source files to compile graphic library glad
# In Emscripten builds, GL bindings are provided by the runtime and glad is not needed.
# Compiling glad on Emscripten leads to null / conflicting GL entry points.
if(NOT EMSCRIPTEN)
    add_library(glad STATIC
        glad/src/glad.c)
else()
    # Create a dummy INTERFACE target so that target_link_libraries(... glad) still works.
    add_library(glad INTERFACE)
endif()
