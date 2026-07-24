// *****************************************************************************
    // start include guard
    #ifndef GAMEPADSINPUT_HPP
    #define GAMEPADSINPUT_HPP
    
    // include console logic headers
    #include "ConsoleLogic/ExternalInterfaces.hpp"
    
    // include C/C++ headers
    #include <map>              // [ C++ STL ] Maps
    #include <string>           // [ C++ STL ] Strings
    
    // include SDL2 headers
    #define SDL_MAIN_HANDLED
    #include "SDL.h"            // [ SDL2 ] Main header
// *****************************************************************************


// =============================================================================
//      DEFINITIONS FOR INPUT MAPPINGS
// =============================================================================


// control mapping for the keyboard
class KeyboardMapping
{
    public:
        
        // d-pad directions
        SDL_Keycode Left, Right, Up, Down;
        
        // buttons
        SDL_Keycode ButtonA, ButtonB, ButtonX, ButtonY;
        SDL_Keycode ButtonL, ButtonR, ButtonStart;
        
        // optional command button
        SDL_Keycode Command;
};

// -----------------------------------------------------------------------------

// possible options for a mapped host device
enum class DeviceTypes
{
    NoDevice,
    Keyboard
};

// -----------------------------------------------------------------------------

// full identification of a host computer device
struct DeviceInfo
{
    // base device info
    DeviceTypes Type;
};


// =============================================================================
//      CLASS FOR GAMEPADS INPUT
// =============================================================================


class GamepadsInput
{
    private:
        
        // all of our available mappings
        KeyboardMapping KeyboardProfile;
        
        // state of the command button for each gamepad (these are optional
        // and not part of the console gamepads so handle them separately)
        bool CommandPressed[ V32::Constants::GamepadPorts ];
        
    public:
        
        // maps {Vircon gamepads} --> {PC devices}
        DeviceInfo MappedGamepads[ V32::Constants::GamepadPorts ];
        
    private:
        
        // specialized event processing functions
        void ProcessKeyDown( SDL_Event Event );
        void ProcessKeyUp( SDL_Event Event );
        
    public:
        
        // instance handling
        GamepadsInput();
       ~GamepadsInput();
        
        // handling control profiles
        void SetDefaultProfiles();
        KeyboardMapping& GetKeyboardProfile();
        
        // handling devices
        void AssignInputDevices();
        
        // processing input events
        void ProcessEvent( SDL_Event Event );
};


// *****************************************************************************
    // end include guard
    #endif
// *****************************************************************************
