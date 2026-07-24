// *****************************************************************************
    // include infrastructure headers
    #include "DesktopInfrastructure/Logger.hpp"
    #include "DesktopInfrastructure/NumericFunctions.hpp"
    #include "DesktopInfrastructure/StringFunctions.hpp"
    #include "DesktopInfrastructure/FilePaths.hpp"
    
    // include console logic headers
    #include "ConsoleLogic/V32Console.hpp"
    
    // include project headers
    #include "EmulatorControl.hpp"
    #include "Settings.hpp"
    #include "GamepadsInput.hpp"
    #include "AudioOutput.hpp"
    #include "VideoOutput.hpp"
    #include "GUI.hpp"
    #include "Languages.hpp"
    #include "Globals.hpp"
    
    // declare used namespaces
    using namespace std;
    using namespace V32;
// *****************************************************************************


// =============================================================================
//      SET DEFAULT EMULATOR SETTINGS
// =============================================================================


// we can't load setting files in this version
void SetDefaultSettings()
{
    // video configuration
    SetWindowZoom1X();
    
    // audio configuration
    Audio.SetMute( false );
    Audio.SetOutputVolume( 1.0 );
    
    // unloaded cartridge
    Console.UnloadCartridge();
    
    // unloaded memory card
    Console.UnloadMemoryCard();
    
    // set keyboard for first gamepad
    Console.SetGamepadConnection( 0, true );
    Gamepads.MappedGamepads[ 0 ].Type = DeviceTypes::Keyboard;
    
    // set no device for the rest of gamepads
    for( int i = 1; i < Constants::GamepadPorts; i++ )
    {
        Console.SetGamepadConnection( i, false );
        Gamepads.MappedGamepads[ i ].Type = DeviceTypes::NoDevice;
    }
    
    // set automatic memory card handling
    Emulator.SetCardHandling( true );
    
    // set default slot for savestates
    SavestatesSlot = 1;
}
