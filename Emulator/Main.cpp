// *****************************************************************************
    // include project headers
    #include "VirconEmulator.hpp"
    #include "StopWatch.hpp"
    #include "OpenGL2DContext.hpp"
    #include "UserActions.hpp"
    #include "Globals.hpp"
    
    // include C/C++ headers
    #include <iostream>           // [ C++ STL ] I/O Streams
    
    // include SDL2 headers
    #include <SDL2/SDL.h>      // [ SDL2 ] Main header
    
    // include OpenAL headers
    #if defined(__APPLE__)
      #include <OpenAL/al.h>      // [ OpenAL ] Main header
      #include <AL/alut.h>        // [ OpenAL ] Utility Toolkit
    #else
      #include <AL/al.h>          // [ OpenAL ] Main header
      #include <AL/alc.h>         // [ OpenAL ] Audio contexts
      #ifndef __EMSCRIPTEN__
        #include <AL/alut.h>      // [ OpenAL ] Utility Toolkit (not available in Emscripten)
      #endif
    #endif
    
    // include Emscripten headers for web
    #ifdef __EMSCRIPTEN__
      #include <emscripten.h>
    #endif
    
    // declare used namespaces
    using namespace std;
// *****************************************************************************


// =============================================================================
//      GLOBAL VARIABLES FOR EMSCRIPTEN LOOP
// =============================================================================

float PendingFrames = 1;
StopWatch Watch;

// =============================================================================
//      MAIN LOOP FUNCTION (for Emscripten)
// =============================================================================

void MainLoop()
{
    // process window events
    SDL_Event Event;
    
    while( SDL_PollEvent( &Event ) )
    {
        // respond to the quit event
        if( Event.type == SDL_QUIT )
        {
            #ifdef __EMSCRIPTEN__
                // In web, we don't exit, just pause
                WindowActive = false;
                Vircon.Pause();
            #else
                GlobalLoopActive = false;
            #endif
        }
        
        // respond to window events
        if( Event.type == SDL_WINDOWEVENT )
        {
            // exit when window is closed
            if( Event.window.event == SDL_WINDOWEVENT_CLOSE )
            {
                #ifdef __EMSCRIPTEN__
                    WindowActive = false;
                    Vircon.Pause();
                #else
                    GlobalLoopActive = false;
                #endif
            }
            
            // on these cases, window updates are paused
            if( Event.window.event == SDL_WINDOWEVENT_MINIMIZED
            ||  Event.window.event == SDL_WINDOWEVENT_HIDDEN
            #ifndef __EMSCRIPTEN__
            ||  Event.window.event == SDL_WINDOWEVENT_FOCUS_LOST
            #endif
            )
            {
                WindowActive = false;
                Vircon.Pause();
            }
            
            // on these cases, window updates are resumed
            if( Event.window.event == SDL_WINDOWEVENT_RESTORED
            ||  Event.window.event == SDL_WINDOWEVENT_SHOWN
            ||  Event.window.event == SDL_WINDOWEVENT_FOCUS_GAINED )   
            {
                WindowActive = true;
                Vircon.Resume();
            }
            
            // on this case, window should be redrawn
            if( Event.window.event == SDL_WINDOWEVENT_EXPOSED )
              OpenGL2D.RenderFrame();
            
            // on any window event (such as lose focus) "stop time"
            Watch.GetStepTime();
        }
        
        // respond to keys being pressed
        if( Event.type == SDL_KEYDOWN && !Event.key.repeat )
        {
            SDL_Keycode Key = Event.key.keysym.sym;
            
            // Escape key toggles emulator pause
            if( Key == SDLK_ESCAPE )
            {
                WindowActive = !WindowActive;
                
                if( WindowActive ) Vircon.Resume();
                else Vircon.Pause();
            }
            
            // Key F5 resets the machine
            if( Key == SDLK_F5 ) Vircon.Reset();
            
            // when CTRL is pressed, process keyboard shortcuts
            bool ControlIsPressed = (SDL_GetModState() & KMOD_CTRL);
            
            if( ControlIsPressed )
            {
                // CTRL+Q = Quit
                if( Key == SDLK_q )
                {
                    #ifdef __EMSCRIPTEN__
                        WindowActive = false;
                        Vircon.Pause();
                    #else
                        GlobalLoopActive = false;
                    #endif
                }
                
                // CTRL+P = Power toggle
                if( Key == SDLK_p )
                {
                    if( Vircon.PowerIsOn )
                      Vircon.PowerOff();
                    else
                      Vircon.PowerOn();
                }
                
                // CTRL+R = Reset
                if( Key == SDLK_r )
                  Vircon.Reset();
                
                // Ctrl+L = Load cartridge
                if( Key == SDLK_l )
                  UserActions::LoadCartridge();
                
                // Ctrl+U = Unload cartridge
                if( Key == SDLK_u )
                  UserActions::UnloadCartridge();
                
                // Ctrl+I = Load memory card
                if( Key == SDLK_i )
                {
                    if( Vircon.HasMemoryCard() )
                      UserActions::LoadMemoryCard();
                };
                
                // Ctrl+O = Unload memory card
                if( Key == SDLK_o )
                {
                    if( Vircon.HasMemoryCard() )
                      UserActions::UnloadMemoryCard();
                }
                
                // Ctrl+F = Fullscreen toggle
                if( Key == SDLK_f )
                {
                    if( OpenGL2D.FullScreen ) OpenGL2D.ExitFullScreen();  
                    else OpenGL2D.SetFullScreen();
                }
                
                // Ctrl+M = Mute toggle
                if( Key == SDLK_m )
                  Vircon.SetMute( !Vircon.IsMuted() );
            }
        }
        
        // LET MACHINE REACT TO THIS MESSAGE
        // (but while window is inactive, events will get ignored)
        
        if( WindowActive )
          Vircon.ProcessEvent( Event );
    }
    
    // update frame only when needed
    if( !WindowActive ) return;
    
    // measure cycle time
    double TimeStep = Watch.GetStepTime();
    PendingFrames += TimeStep * 60.0;
    if( PendingFrames < 0.9 ) return;
    
    while( PendingFrames >= 0.9 )
    {
        // run another frame
        Vircon.RunNextFrame();
        
        // this frame is done
        PendingFrames = max( PendingFrames - 1, 0.0f );
    }
    
    // Show updates on screen
    OpenGL2D.RenderFrame();
}

// =============================================================================
//      MAIN FUNCTION
// =============================================================================


int main( int NumberOfArguments, char* Arguments[] )
{
    try
    {
        // init SDL
        cout << "Initializing SDL" << endl;
        
        Uint32 SDLSubsystems =
        (
            SDL_INIT_VIDEO      |
            SDL_INIT_AUDIO      |
            SDL_INIT_TIMER
            #ifndef __EMSCRIPTEN__
              | SDL_INIT_EVENTS
            #endif
        );
        
        if( SDL_Init( SDLSubsystems ) != 0 )
          throw runtime_error( "Cannot initialize SDL" );
        
        // we need to create a window for SDL to receive any events
        OpenGL2D.CreateOpenGLWindow();
        
        // =======================
        
        // set alpha blending
        cout << "Enabling alpha blending" << endl;
        glEnable( GL_BLEND );
        OpenGL2D.SetBlendingMode( IOPortValues::GPUBlendingMode_Alpha );
        
        //initialize audio
        cout << "Initializing audio" << endl;
        #ifdef __EMSCRIPTEN__
          // Emscripten's OpenAL implementation (Web Audio) creates the device/context
          // automatically on first use. We open it explicitly here so that IsOpenALActive()
          // returns true before SPU.InitializeAudio() is called.
          ALCdevice*  AudioDevice  = alcOpenDevice( nullptr );
          ALCcontext* AudioContext = nullptr;
          if( AudioDevice )
          {
              AudioContext = alcCreateContext( AudioDevice, nullptr );
              if( AudioContext )
                alcMakeContextCurrent( AudioContext );
          }
          if( !IsOpenALActive() )
            throw runtime_error( "Cannot initialize OpenAL (Web Audio API not available)" );
        #else
          alutInit( NULL, NULL );
        #endif
        
        // locating listener
        alListener3f( AL_POSITION, 0, 0, 0 );
        alListenerf( AL_GAIN, 1.0 );
        
        // -----------------------------------------------------------------------------
        
        // load the standard bios from the emulator's local bios folder
        Vircon.LoadBios( "./Bios/StandardBios.v32" );
        
        // turn on Vircon VM
        Vircon.Initialize();
        
        // - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
        
        // program state control
        cout << "Starting main loop" << endl;
        GlobalLoopActive = true;
        
        #ifdef __EMSCRIPTEN__
            // For Emscripten, use the main loop provided by the browser
            emscripten_set_main_loop(MainLoop, 0, 1);
        #else
            // For desktop, use traditional while loop
            while( GlobalLoopActive )
            {
                MainLoop();
            }
            
            // - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
            
            // turn off Vircon VM
            Vircon.Terminate();
            
            // shut down ALUT
            cout << "Terminating audio" << endl;
            #ifndef __EMSCRIPTEN__
              alutExit();
            #endif
            
            // clean-up in reverse order
            cout << "Exiting" << endl;
            OpenGL2D.DestroyOpenGLWindow();
            SDL_Quit();
        #endif
    }
    
    catch( const exception& e )
    {
        // report the error and signal abnormal termination
        cout << "ERROR: " << e.what() << endl;
        return 1;
    }
    
    // signal a successful termination
    return 0;
}
