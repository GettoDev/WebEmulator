<!-- TAB NAVIGATION BAR -->
<div align="center">
  <a href="./README.md">
    <img src="https://img.shields.io/badge/%20README-Overview-71c8ea?style=for-the-badge&labelColor=033d4a" alt="README Tab" />
  </a>
  &nbsp;&nbsp;
  <a href="./PORTING.md">
    <img src="https://img.shields.io/badge/%20PORTING-Technical%20Docs-333333?style=for-the-badge&labelColor=033d4a" alt="PORTING Tab" />
  </a>
</div>

---

| **[ README.md ](./README.md)** *(Active)* | **[ PORTING.md ](./PORTING.md)** |
| :---: | :---: |

---

# Vircon32 Web Emulator — Overview

```
============================================================
         README file for Vircon32 Web Emulator
            (version Desktop) written by Carra
			(Web port) by GettoDev
============================================================
```

What is this?

    This is a web version of the Vircon32 emulator, a 32-bit
    virtual console. It runs directly in your browser without
    any installation required. You only need a modern browser
    with WebGL and Web Audio API support.  

------------------------------------------------------------

Requirements

    To use the emulator you only need:

    - A modern web browser (Chrome, Firefox, Edge or Safari)
    - WebGL support enabled (usually on by default)
    - Web Audio API support (usually on by default)
    - A local web server (required due to browser security
      policies; see "How to run" below for details)

------------------------------------------------------------

How to run

    The web emulator requires a local HTTP server to work
    correctly due to browser security restrictions (CORS and
    SharedArrayBuffer policies). You cannot simply open the
    HTML file directly.

    The easiest ways to start a local server are:

    Python 3:
    ---------
      1. Open a terminal and go to the output folder
      2. Run: python3 -m http.server 8000
      3. Open your browser at: http://localhost:8000

    Python 2:
    ---------
      1. Open a terminal and go to the output folder
      2. Run: python -m SimpleHTTPServer 8000
      3. Open your browser at: http://localhost:8000

    Node.js (http-server):
    ----------------------
      1. Open a terminal and go to the output folder
      2. Run: npx http-server -p 8000
      3. Open your browser at: http://localhost:8000

------------------------------------------------------------

How to load games

    The web emulator does not include any games. You will
    need to load them yourself from your local files.

    To load a game:
    1. Click the "Load ROM (.v32)" button on screen
    2. Select a valid Vircon32 ROM file (*.v32)
    3. The emulator will insert the cartridge automatically
    4. Press CTRL+P to power on the console

    Unlike the desktop version, the cartridge slot is not
    locked while the console is on. You can load a new ROM
    at any time from the interface.

------------------------------------------------------------

Controls

    By default only gamepad 1 is connected and it is mapped
    to the keyboard as follows:

    Player 1:
    ---------
      - D-Pad:        Direction arrow keys
      - Buttons L,R:  Keys 'Q','W'
      - Buttons Y,X:  Keys 'A','S'
      - Buttons B,A:  Keys 'Z','X'
      - Button Start: Key 'Space'
    
    Unlike the desktop version, controls cannot be remapped
    from within the web emulator. There is no EditControls
    companion program in this version.

    Joysticks/gamepads connected to the PC are NOT currently
    supported in the web version.

------------------------------------------------------------

Emulator keyboard shortcuts

    These shortcuts control the emulator itself, not the
    Vircon32 console:

      - ESC:    Show/hide the emulator interface
      - CTRL+P: Power on/off the console
      - CTRL+R: Reset the console
      - CTRL+L: Load a ROM cartridge
      - CTRL+M: Mute/unmute audio
     
------------------------------------------------------------

How to use memory cards

    When a Vircon32 cartridge needs to save or load data
    between sessions, it requires a memory card.

    To load a memory card:
    1. Click the "Load Memory Card" button
    2. Select a valid Vircon32 memory card file (1 MB)
    3. The card will be inserted automatically

    Important differences vs the desktop version:
    - There is no automatic memory card mode. Cards must
      always be loaded and managed manually.
    - Saving to a card modifies it in the browser's memory.
      To keep your progress you must download the card
      after playing, using the "Save Memory Card" button.
    - Memory cards are not persisted between browser sessions
      unless you save and reload them yourself.

------------------------------------------------------------

Differences from the Desktop Emulator

    The web version shares the same core emulation logic as
    the desktop version but has several differences due to
    the nature of the browser environment:

    Not available in web version:
      - EditControls: No control remapping program
      - Joystick/gamepad support
      - Savestates
      - Automatic memory card management
      - Double-click to open ROM files
      - Command button combinations

    Web-only features:
      - Runs entirely in the browser, no installation needed
      - ROM and memory card loading via File API
      - Built with Emscripten, running as WebAssembly (WASM)

------------------------------------------------------------

How to build from source

    The web emulator is compiled from C++ to WebAssembly
    using Emscripten. The build system uses CMake with
    the Ninja generator.

    Requirements:
    -------------
      1. Emscripten SDK  -  https://emscripten.org/docs/getting_started/downloads.html
      2. CMake 3.10 or higher
      3. Ninja build system  -  https://ninja-build.org/

    Windows:
    --------
      1. Install and activate Emscripten SDK:
           emsdk_env.bat
      2. Run the build script:
           .\build_windows.ps1

      The script will automatically:
        - Configure the project with CMake + Ninja
        - Build with emmake
        - Copy all output files to the "output" folder

    Linux / Mac:
    ------------
      1. Install and activate Emscripten SDK:
           source emsdk_env.sh
      2. Give execution permission to the build script:
           chmod +x build_unix.sh
      3. Run the build script:
           ./build_unix.sh

    Manual build (any platform):
    ----------------------------
      1. Create a build directory:
           mkdir build
           cd build
      2. Configure with CMake using Emscripten and Ninja:
           emcmake cmake -G "Ninja" -DCMAKE_BUILD_TYPE=Release ..
      3. Build the project:
           emmake cmake --build .
      4. The output files will be in the build directory:
           Vircon32Web.html
           Vircon32Web.js
           Vircon32Web.wasm
           Vircon32Web.data
           web_interface.js

    Note: The project MUST be built with Emscripten.
    Building with a regular compiler is not supported and
    will result in a CMake error.

------------------------------------------------------------

Project structure

    WebEmulator/
      CMakeLists.txt          - CMake configuration for Emscripten
      index.html              - HTML shell template for Emscripten output
      web_interface.js        - JavaScript bridge between C++ and browser
      build_windows.ps1       - Build script for Windows
      build_unix.sh           - Build script for Linux/Mac
      Emulator/               - Emulator C++ source files
        Main.cpp              - Main loop adapted for Emscripten
        WebFileLoader.cpp     - File loading from browser memory (File API)
        VirconEmulator.cpp    - Core emulator logic
        ...                   - Rest of the emulator source files
      VirconDefinitions/      - Vircon32 console definitions
      ExternalLibraries/      - External libraries (glad)
      Data/                   - Data files
        Bios/
          StandardBios.v32    - Vircon32 standard BIOS (preloaded)
      build/                  - Generated build directory (not in repo)
      output/                 - Final output files after build

------------------------------------------------------------

Troubleshooting

    The emulator does not load:
      - Make sure you are using a local HTTP server
      - Verify that WebGL is enabled in your browser
      - Check the browser developer console for errors

    Audio does not work:
      - Some browsers require a user interaction (click)
        before allowing audio playback
      - Make sure Web Audio API is not blocked in your
        browser settings

    Performance is slow:
      - Close other browser tabs to free up resources
      - Verify that your browser uses hardware-accelerated
        WebGL (software rendering is much slower)
      - Try a different browser (Chrome tends to perform best)

    Build fails:
      - Make sure Emscripten is properly installed and
        activated in your current terminal session
      - Verify that Ninja is installed and in your PATH
      - Check that CMake version is 3.10 or higher

------------------------------------------------------------

License

    This program is free and open source. It is offered under
    the 3-Clause BSD License, which full text is the following:

    Copyright 2021-2026 Carra.
    All rights reserved.

    Redistribution and use in source and binary forms, with or
    without modification, are permitted provided that the
    following conditions are met:

    1. Redistributions of source code must retain the above
    copyright notice, this list of conditions and the following
    disclaimer.

    2. Redistributions in binary form must reproduce the above
    copyright notice, this list of conditions and the following
    disclaimer in the documentation and/or other materials
    provided with the distribution.

    3. Neither the name of the copyright holder nor the names of
    its contributors may be used to endorse or promote products
    derived from this software without specific prior written
    permission.

    THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND
    CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES,
    INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF
    MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
    DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR
    CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
    SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT
    NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
    LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
    HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
    CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR
    OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE,
    EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

------------------------------------------------------------

Links

    Vircon32 official website:  https://www.vircon32.com
    Original repository:        https://github.com/carra1/Vircon32
    Emscripten documentation:   https://emscripten.org/docs/
    Ninja build system:         https://ninja-build.org/
