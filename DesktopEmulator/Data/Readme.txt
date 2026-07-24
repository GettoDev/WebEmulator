============================================================
             README file for Vircon32 emulator
            (version 26.02.18) written by Carra
============================================================

What is this?

    This is an emulator of Vircon32, a 32-bit virtual console,
    for PC. It will allow you to play Vircon32 games on your
    computer.

    The present version is considered finished, save for any
    bugs fixes that may be needed in the future. All console
    features are already supported, and all planned extra
    functions for the emulator (settings, GUI, etc) are done.

------------------------------------------------------------

Installing 
    
    Depending on your operating system, the process to prepare
    the emulator for use will be different:
    
    Windows:
    --------
    Just extract the ZIP into any folder. All needed DLL
    libraries are already included.
      
    Linux:
    --------
    Install the DEB or RPM package (depending on your system)
    using the corresponding commands:
      DEB: (1) Install package: sudo dpkg -i <paquete>
           (2) Install dependencies: sudo apt-get install -f
      RPM: Install package along with its dependencies:
           sudo yum localinstall <paquete>
              
    MacOS:
    --------
    First extract the ZIP into any folder with write permission.
    To install dependencies, the Hombrew package manager is used
    (it can be installed from its website). The commands needed
    to install the dependencies are:
      brew install sdl2
      brew install libpng
    
------------------------------------------------------------

The program window

    Graphic interface is normally hidden, but it will be shown
    when mouse is over the emulator window. It can also be made
    to show manually by pressing the escape key.

    Use the mouse to navigate menus and use their options.

------------------------------------------------------------

How to load games

    This version of the emulator only supports loading a game
    at startup, through command line. For that, pass the path
    to the game ROM as the only program argument.
    
------------------------------------------------------------

Controls

    By default only gamepad 1 is connected and it is mapped to
    the keyboard as follows:

      - D-Pad: Direction arrow keys
      - Buttons L,R: Keys 'Q','W'
      - Buttons Y,X: Keys 'A','S'
      - Buttons B,A: Keys 'Z','X'
      - Button Start: Key 'Return'

------------------------------------------------------------

What's new in version 26.02.18?
  
  - Fixed a bug that preventing setting playback position
    in sound channels.

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
