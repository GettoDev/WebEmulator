<!-- TAB NAVIGATION BAR -->
<div align="center">
  <a href="./README.md">
    <img src="https://img.shields.io/badge/%20README-Overview-333333?style=for-the-badge&labelColor=033d4a" alt="README Tab" />
  </a>
  &nbsp;&nbsp;
  <a href="./PORTING.md">
    <img src="https://img.shields.io/badge/️%20PORTING-Technical%20Docs-71c8ea?style=for-the-badge&labelColor=033d4a" alt="PORTING Tab" />
  </a>
</div>

---

| **[ README.md ](./README.md)** | **[ PORTING.md ](./PORTING.md)** *(Active)* |
| :---: | :---: |

---

# Vircon32 Web Emulator — Porting Notes

Technical document describing all modifications made to port the Vircon32 desktop
emulator to WebAssembly using Emscripten.

---

## Table of Contents

1. [Overview](#1-overview)
2. [Build System (CMakeLists)](#2-build-system-cmakelists)
3. [OpenGL Rendering](#3-opengl-rendering)
4. [Audio (SPU)](#4-audio-spu)
5. [Main Loop and Events](#5-main-loop-and-events)
6. [Cartridge and ROM Loading](#6-cartridge-and-rom-loading)
7. [Memory Card](#7-memory-card)
8. [Web Interface (JS)](#8-web-interface-js)
9. [New Files Exclusive to the Web Version](#9-new-files-exclusive-to-the-web-version)
10. [Removed or Unavailable Features](#10-removed-or-unavailable-features)
11. [Known Errors and Expected Warnings](#11-known-errors-and-expected-warnings)

---

## 1. Overview

The desktop version (`DesktopEmulator/`) compiles with native CMake using
OpenGL 3.0 Core Profile, Dear ImGui, GLFW/SDL2, ALUT, and system libraries.

The web version (`WebEmulator/`) compiles with **Emscripten** to WebAssembly and
runs in the browser. The main constraints driving the changes are:

| Constraint | Impact |
|---|---|
| WebGL 1 / OpenGL ES 2.0 in the browser | No native fixed pipeline (glBegin, glOrtho, etc.) |
| No pthreads by default | The SPU audio thread cannot be launched normally |
| No OS filesystem | ROM loading goes through the JS FileReader API and Emscripten MEMFS |
| No alutInit | The OpenAL context must be opened manually via ALC |
| Browser controls the loop | `main()` cannot have a blocking `while` loop |
| C++ functions must be explicitly exported | Emscripten strips any symbol not marked as exported |

---

## 2. Build System (CMakeLists)

**File:** `WebEmulator/CMakeLists.txt`

### Differences from desktop

The desktop uses a multi-target `CMakeLists.txt` with dependencies on ImGui,
native GLAD, GTK (Linux), OpenAL with ALUT, and OpenGL 3.0.

The web version uses an Emscripten-exclusive `CMakeLists.txt` with the following
changes:

### Added compilation flags

```cmake
-s USE_SDL=2              # SDL2 ported by Emscripten (not from the system)
-lGL                      # Links Emscripten's OpenGL layer
-lopenal                  # OpenAL implemented over Web Audio API
-s WASM=1                 # Output in binary WebAssembly format
-s ALLOW_MEMORY_GROWTH=1  # WASM heap can grow dynamically
-s INITIAL_MEMORY=67108864  # 64 MB of initial memory
-s STACK_SIZE=5242880     # 5 MB stack (needed for large ROMs)
-s LEGACY_GL_EMULATION=1  # Emulates fixed pipeline over WebGL 1
-s GL_FFP_ONLY=1          # Indicates no custom shaders are used (optimizes FFP emulation)
-s MIN_WEBGL_VERSION=1    # Forces WebGL 1 context (not WebGL 2)
-s MAX_WEBGL_VERSION=1    # Same -- FFP emulation only works on WebGL 1
-s MODULARIZE=0           # Module as a global, not an ES module
-s DISABLE_EXCEPTION_CATCHING=0  # Keeps C++ exception support
```

### Functions exported to JavaScript

```cmake
-s EXPORTED_FUNCTIONS=[
    '_main',
    '_loadCartridgeFromPath',
    '_loadMemoryCardFromPath',
    '_loadCartridgeFromMemory',
    '_loadMemoryCardFromMemory',
    '_getMemoryCardSize',
    '_flushMemoryCard'
]
-s EXPORTED_RUNTIME_METHODS=['ccall','cwrap','UTF8ToString','stringToUTF8','FS']
```

Without these declarations Emscripten would remove the functions during optimization.

### GLAD library

In the desktop version, GLAD is a normal static library.
In Emscripten, GLAD leaves GL entry-points null because function pointers are not
loaded the same way as on the desktop. This was resolved by making the `glad` target
an empty `INTERFACE` when compiling with Emscripten:

```cmake
if(NOT EMSCRIPTEN)
    add_library(glad STATIC glad/src/glad.c)
else()
    add_library(glad INTERFACE)  # no-op in web
endif()
```

### Shell-file and preload

```cmake
--shell-file index.html          # HTML that acts as a template
--preload-file Data@/            # Includes Bios/StandardBios.v32 in the bundle
```

---

## 3. OpenGL Rendering

**Files:** `WebEmulator/Emulator/OpenGL2DContext.cpp` /
`WebEmulator/Emulator/OpenGL2DContext.hpp`

### Fundamental pipeline difference

| | Desktop | Web |
|---|---|---|
| OpenGL version | 3.0 Core Profile | WebGL 1 / OpenGL ES 2.0 |
| Pipeline | Programmable (vertex + fragment shaders GLSL 130) | Fixed pipeline emulated with LEGACY_GL_EMULATION |
| Framebuffer | Yes (FBO + texture attachment) | No (direct rendering to canvas) |
| Rendering class | `VideoOutput` | `OpenGL2DContext` |
| Quad rendering | VBO + VAO + glDrawElements | glBegin/glEnd (immediate mode) |
| Multiply color | Uniform in shader | glColor4ub (FFP) |

### OpenGL context

The desktop requests OpenGL 3.0 Core Profile with double buffering. The web version
requests OpenGL ES 2.0 with double buffering mandatory for WebGL:

```cpp
#ifdef __EMSCRIPTEN__
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 2);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 0);
    SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER, 1);  // mandatory in WebGL
#else
    // desktop: OpenGL 2.1 + compatibility profile + no double buffer
    SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER, 0);
#endif
```

### Projection matrix initialization

**Critical bug fixed:** `glOrtho` in Emscripten requires explicitly selecting
the projection matrix before calling it. Without this, the transform is applied
to `GL_MODELVIEW` and nothing renders:

```cpp
// FIX: select GL_PROJECTION before glOrtho
glMatrixMode(GL_PROJECTION);
glLoadIdentity();
glOrtho(0, WindowWidth, WindowHeight, 0, -1, 1);
glMatrixMode(GL_MODELVIEW);
glLoadIdentity();
```

This pattern is repeated in `CreateOpenGLWindow`, `SetFullScreen`, and `ExitFullScreen`.

### GLAD is not initialized in web

```cpp
#ifndef __EMSCRIPTEN__
    if (!gladLoadGLLoader((GLADloadproc)SDL_GL_GetProcAddress))
        throw runtime_error("Error initializing GLAD");
#endif
```

In Emscripten, GL symbols are resolved by the emulation layer, not by GLAD.

---

## 4. Audio (SPU)

**Files:** `WebEmulator/Emulator/VirconSPU.cpp`,
`WebEmulator/Emulator/Main.cpp`

### Audio system differences

| | Desktop | Web |
|---|---|---|
| Audio API | SDL2 AudioDevice (push model) | OpenAL over Web Audio API |
| Threading | `SDL_CreateThread` launches a background playback thread | No thread -- buffer management from main loop |
| Initialization | `alutInit(NULL, NULL)` | Manual `alcOpenDevice` + `alcCreateContext` + `alcMakeContextCurrent` |

### Audio thread disabled in web

Emscripten does not support `SDL_CreateThread` without SharedArrayBuffer/pthreads
enabled (which require COOP/COEP headers that complicate hosting). The thread was
disabled and buffer management was moved to the main loop:

```cpp
// VirconSPU::LaunchPlaybackThread
#ifdef __EMSCRIPTEN__
    PlaybackThread = nullptr;  // no-op in web
#else
    PlaybackThread = SDL_CreateThread(SPUPlaybackThread, "Playback", this);
#endif
```

```cpp
// VirconSPU::ChangeFrame -- additional calls in web
#ifdef __EMSCRIPTEN__
    QueueFilledBuffers();
    UnqueuePlayedBuffers();
#endif
```

### OpenAL initialization in web

```cpp
#ifdef __EMSCRIPTEN__
    ALCdevice* AudioDevice = alcOpenDevice(nullptr);
    ALCcontext* AudioContext = alcCreateContext(AudioDevice, nullptr);
    alcMakeContextCurrent(AudioContext);
    // verify it is active before continuing
    if (!IsOpenALActive())
        throw runtime_error("Cannot initialize OpenAL");
#else
    alutInit(NULL, NULL);
#endif
```

---

## 5. Main Loop and Events

**File:** `WebEmulator/Emulator/Main.cpp`

### Blocking loop vs. browser callback

The desktop uses a traditional `while(GlobalLoopActive)` loop. In the browser a
blocking loop freezes the tab. Emscripten requires yielding control to the browser
on every frame:

```cpp
#ifdef __EMSCRIPTEN__
    emscripten_set_main_loop(MainLoop, 0, 1);
    // 0 = no FPS limit (uses browser requestAnimationFrame)
    // 1 = simulate_infinite_loop (stops execution here)
#else
    while(GlobalLoopActive) { MainLoop(); }
    Vircon.Terminate();
    SDL_Quit();
#endif
```

### Differences in event handling

| Event | Desktop | Web |
|---|---|---|
| SDL_QUIT | Terminates the process | Pauses emulation |
| FOCUS_LOST | Pause + switch to SDL_WaitEvent | Not applicable (browser tab visibility API) |
| CTRL+Q | Closes the application | Pauses emulation |

### SDL_INIT_EVENTS

```cpp
SDL_INIT_VIDEO | SDL_INIT_AUDIO | SDL_INIT_TIMER
#ifndef __EMSCRIPTEN__
    | SDL_INIT_EVENTS  // in web the event loop is managed by the browser
#endif
```

### Absence of ImGui, joysticks, and GUI

The desktop version has the entire visual interface inside the main loop
(ImGui, menus, dialogs). The web version removes all of this -- the interface is
exclusively the HTML/JS of the page.

---

## 6. Cartridge and ROM Loading

**Files:** `WebEmulator/Emulator/WebFileLoader.cpp`,
`WebEmulator/Emulator/VirconEmulator.cpp`

### The problem

The browser cannot read files from the user's filesystem directly. Files must go
through `<input type="file">` in JavaScript, be read with `FileReader`, and then
transferred to WASM.

### Implemented solution: MEMFS + path

The preferred method for large ROMs is to write the file to Emscripten's virtual
filesystem (MEMFS) and then load it by path:

**JavaScript (`web_interface.js`):**
```javascript
function handleFileSelect(event) {
    const reader = new FileReader();
    reader.onload = function(e) {
        const uint8Array = new Uint8Array(e.target.result);
        FS.createPath('/', 'uploads', true, true);
        FS.writeFile('/uploads/cartridge.v32', uint8Array);
        Module.ccall('loadCartridgeFromPath', null, ['string'], ['/uploads/cartridge.v32']);
    };
    reader.readAsArrayBuffer(file);
}
```

**C++ (`WebFileLoader.cpp`):**
```cpp
EMSCRIPTEN_KEEPALIVE
void loadCartridgeFromPath(const char* path) {
    PrepareCartridgeSlot();       // power off console, remove previous cartridge
    Vircon.LoadCartridge(path);   // uses the normal file implementation
    Vircon.PowerOn();
    Vircon.Resume();
    NotifyStatus("Cartridge loaded and console powered on");
}
```

### Fix for section offset in LoadCartridgeFromMemory

The original `LoadCartridgeFromMemory` implementation calculated the offset for
each texture/sound as `base + i * sizeof(Header)`, which is incorrect because
each section has variable size (header + data). It was rewritten with a sequential
cursor:

```cpp
// CORRECT: cursor that advances with each section read
uint32_t VideoCursor = ROMHeader.VideoROMLocation.StartOffset;
for (unsigned i = 0; i < ROMHeader.NumberOfTextures; i++) {
    TextureFileHeader hdr;
    memcpy(&hdr, Data + VideoCursor, sizeof(hdr));
    VideoCursor += sizeof(hdr);
    // ... load pixels ...
    VideoCursor += TexturePixels * sizeof(VirconWord);
}
```

### NumberOfTextures and NumberOfSounds

In the desktop version these values were set, but in the web version they were
missing from the `CartridgeController` assignment. They were added in `LoadCartridge`
and reset to 0 in `UnloadCartridge`.

---

## 7. Memory Card

**Files:** `WebEmulator/Emulator/WebFileLoader.cpp`,
`WebEmulator/web_interface.js`

### Loading

Same as cartridges: the file is written to MEMFS at `/uploads/memorycard.memc`
and loaded via `loadMemoryCardFromPath`.

### Saving (download to user)

The memory card is modified in RAM during emulation and saved periodically to
MEMFS via `PendingSave`. To download it to the user:

1. `flushMemoryCard()` was exported, which forces an immediate `SaveContents` to MEMFS.
2. JavaScript reads the file from MEMFS and generates a download link:

```javascript
function saveMemoryCard() {
    Module.ccall('flushMemoryCard', null, [], []);
    const data = FS.readFile('/uploads/memorycard.memc', { encoding: 'binary' });
    const url = URL.createObjectURL(new Blob([data]));
    const a = document.createElement('a');
    a.href = url;
    a.download = 'memorycard.memc';
    a.click();
}
```

### LoadMemoryCardFromMemory fix

The original implementation assumed exactly 1 MB without a signature. It was
corrected to validate the size with signature (`8 + MemoryCardSize * 4`), verify
the `V32-MEMC` signature, and assign `CardSavePath` so that `flushMemoryCard`
works correctly.

---

## 8. Web Interface (JS)

**Files:** `WebEmulator/web_interface.js`, `WebEmulator/index.html`

### Module object configuration

`web_interface.js` must be loaded **before** `Vircon32Web.js` so that the `Module`
object is configured before the WASM runtime starts:

```javascript
var Module = (typeof Module !== 'undefined') ? Module : {};

Module.print    = (text) => console.log('[Vircon32]', text);
Module.printErr = (text) => console.error('[Vircon32]', text);
Module.onAbort  = (reason) => { /* shows error in the UI */ };
Module.GL_MAX_TEXTURE_IMAGE_UNITS = 1;
Module.canvas   = document.getElementById('canvas');
Module.onRuntimeInitialized = function() { setupFileUpload(); };
```

### GL_MAX_TEXTURE_IMAGE_UNITS = 1 -- WebGL vertex attrib fix

> **Critical bug fixed.** Emscripten's legacy GL emulation layer (`libglemu.js`) loops over texture unit attributes up to `GL_MAX_TEXTURE_IMAGE_UNITS`. By default, browsers report 16 max texture units for WebGL 1, causing Emscripten to attempt accessing attribute indices up to 18 (e.g. `TEXTURE0 + 15`). Because WebGL 1 caps `MAX_VERTEX_ATTRIBS` at 16 (indices 0–15), this flooded the console with thousands of `GL_INVALID_VALUE: glVertexAttrib4f: Index must be less than MAX_VERTEX_ATTRIBS` errors per frame.

To fix this, two connected changes were implemented:

**1. `INCOMING_MODULE_JS_API` in `CMakeLists.txt`:**

```cmake
"SHELL:-s INCOMING_MODULE_JS_API=['ENVIRONMENT','arguments','canvas','dynamicLibraries','elementPointerLock','instantiateWasm','locateFile','monitorRunDependencies','noExitRuntime','noInitialRun','onAbort','onExit','onRuntimeInitialized','postRun','preInit','preRun','print','printErr','setStatus','statusMessage','stderr','stdin','stdout','thisProgram','wasm','websocket','GL_MAX_TEXTURE_IMAGE_UNITS']"
```

By default, Emscripten optimizes away any `Module['...']` property read that is not explicitly declared in `INCOMING_MODULE_JS_API`. Adding `'GL_MAX_TEXTURE_IMAGE_UNITS'` ensures Emscripten emits `var maxTextureUnits = Module['GL_MAX_TEXTURE_IMAGE_UNITS'] || ...` in the generated WebAssembly JS wrapper.

**2. `Module.GL_MAX_TEXTURE_IMAGE_UNITS = 1` in `web_interface.js`:**

```javascript
Module.GL_MAX_TEXTURE_IMAGE_UNITS = 1;
```

Because Vircon32 only uses 1 texture unit at a time, setting this value to 1 clamps `GLImmediate.MAX_TEXTURES` to 1, preventing the attribute index from ever exceeding 3 (`POSITION=0`, `NORMAL=1`, `COLOR=2`, `TEXTURE0=3`), fully inside WebGL 1's bounds.

### How to build

```powershell
# From the WebEmulator/ folder with Emscripten activated:
.\build_windows.ps1

# To serve the result:
python serve.py         # serves output/ at http://localhost:8000
```

### Recompiling vs. editing output/

Files in `output/` are generated by the build. Manual changes to
`output/Vircon32Web.html` are **lost on recompile**.
For permanent changes, modify `index.html` (the source shell-file) and recompile,
or reapply the changes post-build.

---

