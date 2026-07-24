// *****************************************************************************
    // start include guard
    #ifndef SETTINGS_HPP
    #define SETTINGS_HPP
    
    // include common Vircon headers
    #include "../VirconDefinitions/Constants.hpp"
    
    // include C/C++ headers
    #include <string>           // [ C++ STL ] Strings
    #include <map>              // [ C++ STL ] Maps
    
    // include SDL2 headers
    #define SDL_MAIN_HANDLED
    #include "SDL.h"            // [ SDL2 ] Main header
// *****************************************************************************


// =============================================================================
//      SET DEFAULT EMULATOR SETTINGS
// =============================================================================


void SetDefaultSettings();


// *****************************************************************************
    // end include guard
    #endif
// *****************************************************************************

