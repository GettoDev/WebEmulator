// *****************************************************************************
    // include console logic headers
    #include "ConsoleLogic/V32Console.hpp"
    
    // include infrastructure headers
    #include "DesktopInfrastructure/Logger.hpp"
    
    // include emulator headers
    #include "GamepadsInput.hpp"
    #include "GUI.hpp"
    #include "Globals.hpp"
    
    // include C/C++ headers
    #include <stdexcept>        // [ C++ STL ] Exceptions
    #include <iostream>         // [ C++ STL ] I/O Streams
    #include <set>              // [ C++ STL ] Sets
    #include <climits>          // [ ANSI C ] Numeric limits
    
    // declare used namespaces
    using namespace std;
    using namespace V32;
// *****************************************************************************


// =============================================================================
//      GAMEPADS INPUT: INSTANCE HANDLING
// =============================================================================


GamepadsInput::GamepadsInput()
{
    SetDefaultProfiles();
    
    // command buttons are all initially unpressed
    for( int Gamepad = 0; Gamepad < Constants::GamepadPorts; Gamepad++ )
      CommandPressed[ Gamepad ] = false;
}

// -----------------------------------------------------------------------------

GamepadsInput::~GamepadsInput()
{
}


// =============================================================================
//      GAMEPADS INPUT: HANDLING CONTROL PROFILES
// =============================================================================


void GamepadsInput::SetDefaultProfiles()
{
    // set the default keyboard profile
    KeyboardProfile.Left = SDLK_LEFT;
    KeyboardProfile.Right = SDLK_RIGHT;
    KeyboardProfile.Up = SDLK_UP;
    KeyboardProfile.Down = SDLK_DOWN;
    
    KeyboardProfile.ButtonA = SDLK_x;
    KeyboardProfile.ButtonB = SDLK_z;
    KeyboardProfile.ButtonX = SDLK_s;
    KeyboardProfile.ButtonY = SDLK_a;
    KeyboardProfile.ButtonL = SDLK_q;
    KeyboardProfile.ButtonR = SDLK_w;
    
    KeyboardProfile.ButtonStart = SDLK_RETURN;
    
    // by default Command button is not used
    KeyboardProfile.Command = -1;
}

// -----------------------------------------------------------------------------

KeyboardMapping& GamepadsInput::GetKeyboardProfile()
{
    return KeyboardProfile;
}


// =============================================================================
//      GAMEPADS INPUT: HANDLING DEVICES
// =============================================================================


void GamepadsInput::AssignInputDevices()
{
    bool IsKeyboardUsed = false;
    
    // update mappings for gamepads
    for( int Gamepad = 0; Gamepad < Constants::GamepadPorts; Gamepad++ )
    {
        DeviceInfo* GamepadDevice = &MappedGamepads[ Gamepad ];
        
        // preemptively disconnect the gamepad
        Console.SetGamepadConnection( Gamepad, false );
        
        // process non-joystick devices
        if( GamepadDevice->Type == DeviceTypes::NoDevice )
          continue;
        
        if( GamepadDevice->Type == DeviceTypes::Keyboard )
        {
            // allow only for 1 gamepad to use the keyboard
            if( IsKeyboardUsed )
              GamepadDevice->Type = DeviceTypes::NoDevice;
              
            else
            {
                IsKeyboardUsed = true;
                Console.SetGamepadConnection( Gamepad, true );
            }
            
            continue;
        }
    }
}


// =============================================================================
//      GAMEPADS INPUT: PROCESSING INPUT EVENTS
// =============================================================================


void GamepadsInput::ProcessEvent( SDL_Event Event )
{
    switch( Event.type )
    {
        case SDL_KEYDOWN:
            ProcessKeyDown( Event );
            break;
        case SDL_KEYUP:
            ProcessKeyUp( Event );
            break;
    }
}

// -----------------------------------------------------------------------------

void GamepadsInput::ProcessKeyDown( SDL_Event Event )
{
    // don't process automatic key retriggers
    if( Event.key.repeat ) return;
    
    // ignore keypresses when control is pressed,
    // so that keyboard shortcuts will not interfere
    SDL_Keycode KeyCode = Event.key.keysym.sym;
    bool ControlIsPressed = (SDL_GetModState() & KMOD_CTRL);
    if( ControlIsPressed ) return;
    
    // in other cases process the key normally
    for( int Gamepad = 0; Gamepad < Constants::GamepadPorts; Gamepad++ )
    {
        // non-connected gamepads are ignored
        if( !Console.HasGamepad( Gamepad ) )
          continue;
        
        // check if mapped device is the keyboard
        if( MappedGamepads[ Gamepad ].Type != DeviceTypes::Keyboard )
          continue;
        
        // if the command button is mapped to a key
        // check it before any regular controls
        if( KeyboardProfile.Command >= 0 )
          if( KeyCode == KeyboardProfile.Command )
            CommandPressed[ Gamepad ] = true;
        
        // check the mapped keys for directions
        if( KeyCode == KeyboardProfile.Left )
          Console.SetGamepadControl( Gamepad, GamepadControls::Left, true );
          
        if( KeyCode == KeyboardProfile.Right )
          Console.SetGamepadControl( Gamepad, GamepadControls::Right, true );
          
        if( KeyCode == KeyboardProfile.Up )
          Console.SetGamepadControl( Gamepad, GamepadControls::Up, true );
          
        if( KeyCode == KeyboardProfile.Down )
          Console.SetGamepadControl( Gamepad, GamepadControls::Down, true );
          
        // when command is pressed, check only for button combinations
        // (regular button presses are ignored until command is released)
        if( CommandPressed[ Gamepad ] )
        {
            // hold Command + press X = Reset
            if( KeyCode == KeyboardProfile.ButtonX )
              Console.Reset();
            
            // hold Command + press Start = Quit emulator
            if( KeyCode == KeyboardProfile.ButtonStart )
              GlobalLoopActive = false;
        }
        
        // only when command is not pressed check the mapped keys for buttons
        else
        {
            if( KeyCode == KeyboardProfile.ButtonA )
              Console.SetGamepadControl( Gamepad, GamepadControls::ButtonA, true );
            
            if( KeyCode == KeyboardProfile.ButtonB )
              Console.SetGamepadControl( Gamepad, GamepadControls::ButtonB, true );
            
            if( KeyCode == KeyboardProfile.ButtonX )
              Console.SetGamepadControl( Gamepad, GamepadControls::ButtonX, true );
            
            if( KeyCode == KeyboardProfile.ButtonY )
              Console.SetGamepadControl( Gamepad, GamepadControls::ButtonY, true );
              
            if( KeyCode == KeyboardProfile.ButtonL )
              Console.SetGamepadControl( Gamepad, GamepadControls::ButtonL, true );
            
            if( KeyCode == KeyboardProfile.ButtonR )
              Console.SetGamepadControl( Gamepad, GamepadControls::ButtonR, true );
            
            if( KeyCode == KeyboardProfile.ButtonStart )
              Console.SetGamepadControl( Gamepad, GamepadControls::ButtonStart, true );
        }
    }
}

// -----------------------------------------------------------------------------

void GamepadsInput::ProcessKeyUp( SDL_Event Event )
{
    // ignore keypresses when control is pressed,
    // so that keyboard shortcuts will not interfere
    SDL_Keycode KeyCode = Event.key.keysym.sym;
    bool ControlIsPressed = (SDL_GetModState() & KMOD_CTRL);
    if( ControlIsPressed ) return;
    
    // in other cases process the key normally
    for( int Gamepad = 0; Gamepad < Constants::GamepadPorts; Gamepad++ )
    {
        // non-connected gamepads are ignored
        if( !Console.HasGamepad( Gamepad ) )
          continue;
        
        // check if mapped device is the keyboard
        if( MappedGamepads[ Gamepad ].Type != DeviceTypes::Keyboard )
          continue;
        
        // if the command button is mapped to a key
        // check it before any regular controls
        if( KeyboardProfile.Command >= 0 )
          if( KeyCode == KeyboardProfile.Command )
            CommandPressed[ Gamepad ] = false;
        
        // check the mapped keys for directions
        if( KeyCode == KeyboardProfile.Left )
          Console.SetGamepadControl( Gamepad, GamepadControls::Left, false );
          
        if( KeyCode == KeyboardProfile.Right )
          Console.SetGamepadControl( Gamepad, GamepadControls::Right, false );
          
        if( KeyCode == KeyboardProfile.Up )
          Console.SetGamepadControl( Gamepad, GamepadControls::Up, false );
          
        if( KeyCode == KeyboardProfile.Down )
          Console.SetGamepadControl( Gamepad, GamepadControls::Down, false );
          
        // only when command is not pressed check the mapped buttons for buttons
        if( !CommandPressed[ Gamepad ] )
        {
            // check the mapped keys for buttons
            if( KeyCode == KeyboardProfile.ButtonA )
              Console.SetGamepadControl( Gamepad, GamepadControls::ButtonA, false );
            
            if( KeyCode == KeyboardProfile.ButtonB )
              Console.SetGamepadControl( Gamepad, GamepadControls::ButtonB, false );
            
            if( KeyCode == KeyboardProfile.ButtonX )
              Console.SetGamepadControl( Gamepad, GamepadControls::ButtonX, false );
            
            if( KeyCode == KeyboardProfile.ButtonY )
              Console.SetGamepadControl( Gamepad, GamepadControls::ButtonY, false );
              
            if( KeyCode == KeyboardProfile.ButtonL )
              Console.SetGamepadControl( Gamepad, GamepadControls::ButtonL, false );
            
            if( KeyCode == KeyboardProfile.ButtonR )
              Console.SetGamepadControl( Gamepad, GamepadControls::ButtonR, false );
            
            if( KeyCode == KeyboardProfile.ButtonStart )
              Console.SetGamepadControl( Gamepad, GamepadControls::ButtonStart, false );
        }
    }
}
