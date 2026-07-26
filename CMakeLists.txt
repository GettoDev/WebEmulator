# CMakeLists.txt for Vircon32 Web Emulator (Emscripten)
cmake_minimum_required(VERSION 3.10)

# Define the project
project("Vircon32Web" LANGUAGES C CXX)

# Set version
set(PROJECT_VERSION_MAJOR 1)
set(PROJECT_VERSION_MINOR 0)
set(PROJECT_VERSION_PATCH 0)

# Set binary name
set(EMULATOR_BINARY_NAME "Vircon32Web")

# Check if we are building with Emscripten
if(NOT EMSCRIPTEN)
    message(FATAL_ERROR "This project must be built with Emscripten. Please use emcmake cmake.")
endif()

# -----------------------------------------------------
#   BUILD FLAGS / CONFIGURATION
# -----------------------------------------------------

# Set compilation flags for C and C++
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11 -Wall -Wextra -Wno-unused-parameter -s USE_SDL=2")
set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -Wall -Wextra -Wno-unused-parameter -s USE_SDL=2")

# -----------------------------------------------------
#   FOLDERS FOR INCLUDES
# -----------------------------------------------------

set(EMULATOR_DIR "Emulator/")
set(DEFINITIONS_DIR "VirconDefinitions/")
set(LIBRARIES_DIR "ExternalLibraries/")
set(DATA_DIR "Data/")

include_directories(
    ${EMULATOR_DIR}
    ${DEFINITIONS_DIR}
    ${LIBRARIES_DIR}
    ${LIBRARIES_DIR}/glad/include/
)

# -----------------------------------------------------
#   SOURCE FILES
# -----------------------------------------------------

set(EMULATOR_SRC
    ${EMULATOR_DIR}/Globals.cpp
    ${EMULATOR_DIR}/Main.cpp
    ${EMULATOR_DIR}/OpenGL2DContext.cpp
    ${EMULATOR_DIR}/StopWatch.cpp
    ${EMULATOR_DIR}/UserActions.cpp
    ${EMULATOR_DIR}/VirconBuses.cpp
    ${EMULATOR_DIR}/VirconCartridgeController.cpp
    ${EMULATOR_DIR}/VirconCPU.cpp
    ${EMULATOR_DIR}/VirconCPUProcessors.cpp
    ${EMULATOR_DIR}/VirconEmulator.cpp
    ${EMULATOR_DIR}/VirconGPU.cpp
    ${EMULATOR_DIR}/VirconGPUWriters.cpp
    ${EMULATOR_DIR}/VirconGamepadController.cpp
    ${EMULATOR_DIR}/VirconMemory.cpp
    ${EMULATOR_DIR}/VirconMemoryCardController.cpp
    ${EMULATOR_DIR}/VirconNullController.cpp
    ${EMULATOR_DIR}/VirconRNG.cpp
    ${EMULATOR_DIR}/VirconSPU.cpp
    ${EMULATOR_DIR}/VirconSPUThread.cpp
    ${EMULATOR_DIR}/VirconSPUWriters.cpp
    ${EMULATOR_DIR}/VirconTimer.cpp
    ${EMULATOR_DIR}/WebFileLoader.cpp
    ${DEFINITIONS_DIR}/VirconROMFormat.cpp
)

# -----------------------------------------------------
#   EXTERNAL LIBRARIES
# -----------------------------------------------------

add_subdirectory(${LIBRARIES_DIR})

# -----------------------------------------------------
#   EXECUTABLE
# -----------------------------------------------------

add_executable(${EMULATOR_BINARY_NAME} ${EMULATOR_SRC})
set_property(TARGET ${EMULATOR_BINARY_NAME} PROPERTY CXX_STANDARD 11)

# Link libraries
target_link_libraries(${EMULATOR_BINARY_NAME}
    glad
)

# -----------------------------------------------------
#   EMSCRIPTEN SPECIFIC SETTINGS
# -----------------------------------------------------

# Output is an HTML file with the Emscripten shell template
set(CMAKE_EXECUTABLE_SUFFIX ".html")

# Emscripten linker flags
# LEGACY_GL_EMULATION + GL_FFP_ONLY: emulate desktop fixed-pipeline GL
# (glBegin/glOrtho/glPushMatrix/...) on top of WebGL 1.
# MIN_WEBGL_VERSION=1 + MAX_WEBGL_VERSION=1: force WebGL 1 context — the FFP
# emulation layer only works on WebGL 1. WebGL 2 contexts drop fixed-pipeline.
target_link_options(${EMULATOR_BINARY_NAME} PRIVATE
    "SHELL:-s USE_SDL=2"
    "-lopenal"
    "-lGL"
    "SHELL:-s WASM=1"
    "SHELL:-s ALLOW_MEMORY_GROWTH=1"
    "SHELL:-s INITIAL_MEMORY=67108864"
    "SHELL:-s STACK_SIZE=5242880"
    "SHELL:-s LEGACY_GL_EMULATION=1"
    "SHELL:-s GL_FFP_ONLY=1"
    "SHELL:-s INCOMING_MODULE_JS_API=['ENVIRONMENT','arguments','canvas','dynamicLibraries','elementPointerLock','instantiateWasm','locateFile','monitorRunDependencies','noExitRuntime','noInitialRun','onAbort','onExit','onRuntimeInitialized','postRun','preInit','preRun','print','printErr','setStatus','statusMessage','stderr','stdin','stdout','thisProgram','wasm','websocket','GL_MAX_TEXTURE_IMAGE_UNITS']"
    "SHELL:-s MIN_WEBGL_VERSION=1"
    "SHELL:-s MAX_WEBGL_VERSION=1"
    "SHELL:-s DISABLE_EXCEPTION_CATCHING=0"
    "SHELL:-s MODULARIZE=0"
    "SHELL:-s EXPORTED_FUNCTIONS=['_main','_loadCartridgeFromPath','_loadMemoryCardFromPath','_loadCartridgeFromMemory','_loadMemoryCardFromMemory','_getMemoryCardSize','_flushMemoryCard']"
    "SHELL:-s EXPORTED_RUNTIME_METHODS=['ccall','cwrap','UTF8ToString','stringToUTF8','FS']"
    "SHELL:--shell-file ${CMAKE_CURRENT_SOURCE_DIR}/index.html"
)

# Preload assets (mapping Data directory contents to the root of the virtual filesystem)
target_link_options(${EMULATOR_BINARY_NAME} PRIVATE
    "SHELL:--preload-file ${CMAKE_CURRENT_SOURCE_DIR}/Data@/"
)

# Copy web_interface.js to build directory
add_custom_command(TARGET ${EMULATOR_BINARY_NAME} POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy
    ${CMAKE_CURRENT_SOURCE_DIR}/web_interface.js
    ${CMAKE_BINARY_DIR}/web_interface.js
)

# Set output directory
set_target_properties(${EMULATOR_BINARY_NAME} PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}"
)

# -----------------------------------------------------
#   INSTALLATION
# -----------------------------------------------------

install(TARGETS ${EMULATOR_BINARY_NAME}
    RUNTIME
    DESTINATION .
)
