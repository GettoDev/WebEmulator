// *****************************************************************************
    // include console logic headers
    #include "ConsoleLogic/V32Console.hpp"
    
    // include infrastructure headers
    #include "DesktopInfrastructure/FilePaths.hpp"
    #include "DesktopInfrastructure/Logger.hpp"
    
    // include emulator headers
    #include "GUI.hpp"
    #include "EmulatorControl.hpp"
    #include "GamepadsInput.hpp"
    #include "VideoOutput.hpp"
    #include "AudioOutput.hpp"
    #include "Globals.hpp"
    #include "Settings.hpp"
    #include "Languages.hpp"
    
    // include C/C++ headers
    #include <time.h>               // [ ANSI C ] Time and date
    #include <stdexcept>            // [ C++ STL ] Exceptions
    
    // include libpng headers
    #include <png.h>                // [ libpng ] Main header
    
    // include imgui headers
    #include <imgui/imgui.h>                // [ Dear ImGui ] Main header
    #include <imgui/imgui_impl_sdl.h>       // [ Dear ImGui ] SDL2 backend header
    #include <imgui/imgui_impl_opengl3.h>   // [ Dear ImGui ] OpenGL 3 backend header
    
    // detection of Windows
    #if defined(__WIN32__) || defined(_WIN32) || defined(_WIN64)
      #define WINDOWS_OS
    #endif
    
    // declare used namespaces
    using namespace std;
    using namespace V32;
// *****************************************************************************


// =============================================================================
//      DELAYED MESSAGE BOX FUNCTIONS
// =============================================================================
// these are needed because in full screen, showing a
// regular message box may blocks the program. This is
// avoided by waiting until the frame finishes drawing

bool MessageBoxPending = false;
Uint32 MessageBoxFlags;
char MessageBoxTitle[ 128 ];
char MessageBoxMessage[ 2048 ];

// -----------------------------------------------------------------------------

void DelayedMessageBox( Uint32 Flags, const char *Title, const char *Message )
{
    MessageBoxPending = true;
    MessageBoxFlags = Flags;
    strncpy( MessageBoxTitle, Title, sizeof(MessageBoxTitle)-1 );
    strncpy( MessageBoxMessage, Message, sizeof(MessageBoxMessage)-1 );
}

// -----------------------------------------------------------------------------

void ShowDelayedMessageBox()
{
    if( !MessageBoxPending )
      return;
    
    // check current state to restore later
    bool WasRunning = Emulator.IsPowerOn() && !Emulator.IsPaused();
    
    if( WasRunning )
      Emulator.Pause();
    
    SDL_ShowSimpleMessageBox( MessageBoxFlags, MessageBoxTitle, MessageBoxMessage, nullptr );
    MessageBoxPending = false;
    
    // restore previous state if needed
    if( WasRunning )
      Emulator.Resume();
}

// -----------------------------------------------------------------------------

void CancelDelayedMessageBox()
{
    MessageBoxPending = false;
}


// =============================================================================
//      ADDITIONAL GUI FUNCTIONS
// =============================================================================
// these are some more complex GUI functionalities that
// are better understood as their own separate functions


void SetWindowZoom1X()
{
    LOG( "Setting window zoom x1" );
    
    // pause emulation at window events to
    // ensure sound is restored after them
    bool WasRunning = Emulator.IsPowerOn() && !Emulator.IsPaused();
    
    if( WasRunning )
      Emulator.Pause();
    
    // set the zoom
    Video.SetWindowZoom( 1 );
    
    // scale ImGui
    ImGui::GetIO().FontGlobalScale = Video.GetWindowZoom();
    
    // resume emulation if needed
    if( WasRunning )
      Emulator.Resume();
}

// -----------------------------------------------------------------------------

// determine the path of the memory card corresponding to a given,
// game file, taking into account the emulator's card directory
// (this is only used when memory card handling is set to automatic)
string GetAutomaticMemoryCardPath( const string& CartridgePath )
{
    // step 1: determine the emulator's card folder
    string CardsFolder = EmulatorFolder + "Cards";
    
    // step 2: isolate file name and replace extension
    string CartridgeFileName = GetPathFileName( CartridgePath );
    LOG( "CartridgePath = \"" + CartridgePath );
    LOG( "CartridgeFileName = \"" + CartridgeFileName );
    string CardFileName = ReplaceFileExtension( CartridgeFileName, "memc" );
    LOG( "CardFileName = \"" + CardFileName );
    
    // step 3: form the full path
    return CardsFolder + PathSeparator + CardFileName;
}


// =============================================================================
//      ENCAPSULATED GUI FUNCTIONS
// =============================================================================
// some of the GUI functions involve files and can potentially
// throw exceptions, so we have to add some logic to wrap them
// in try/catch blocks and report errors


void GUI_CreateMemoryCard( string MemoryCardPath )
{
    try
    {
        if( !MemoryCardPath.empty() )
        {
            // ensure path has the proper extension
            if( GetFileExtension( MemoryCardPath ) != "memc" )
              MemoryCardPath += ".memc";
            
            // create the card
            Console.CreateMemoryCard( MemoryCardPath );
            
            // report only if we requested this manually
            if( Emulator.IsCardHandlingAuto() )
              return;
            
            DelayedMessageBox
            (
                SDL_MESSAGEBOX_INFORMATION,
                Texts( TextIDs::Dialogs_Done ),
                Texts( TextIDs::Dialogs_CardCreated_Label )
            );
        }
    }
    
    catch( const exception& e )
    {
        string Message = Texts( TextIDs::Errors_CreateCard_Label ) + string(e.what());
        DelayedMessageBox( SDL_MESSAGEBOX_ERROR, "Error", Message.c_str() );
    }
}

// -----------------------------------------------------------------------------

void GUI_UnloadMemoryCard()
{
    try
    {
        Console.UnloadMemoryCard();
    }
    
    catch( const exception& e )
    {
        string Message = Texts( TextIDs::Errors_UnloadCard_Label ) + string(e.what());
        DelayedMessageBox( SDL_MESSAGEBOX_ERROR, "Error", Message.c_str() );
    }
}

// -----------------------------------------------------------------------------

void GUI_LoadMemoryCard( string MemoryCardPath )
{
    try
    {
        if( !MemoryCardPath.empty() )
        {
            Console.LoadMemoryCard( MemoryCardPath );
        }
    }
    
    catch( const exception& e )
    {
        string Message = Texts( TextIDs::Errors_LoadCard_Label ) + string(e.what());
        DelayedMessageBox( SDL_MESSAGEBOX_ERROR, "Error", Message.c_str() );
    }
}

// -----------------------------------------------------------------------------

void GUI_ChangeMemoryCard( string MemoryCardPath )
{
    try
    {
        if( !MemoryCardPath.empty() )
        {
            Console.UnloadMemoryCard();
            Console.LoadMemoryCard( MemoryCardPath );
        }
    }
    
    catch( const exception& e )
    {
        string Message = Texts( TextIDs::Errors_ChangeCard_Label ) + string(e.what());
        DelayedMessageBox( SDL_MESSAGEBOX_ERROR, "Error", Message.c_str() );
    }
}

// -----------------------------------------------------------------------------

void GUI_AutoUpdateMemoryCard()
{
    try
    {
        // unload any previous card
        if( Console.HasMemoryCard() )
          Console.UnloadMemoryCard();
        
        // if there is no game, just remove any loaded card
        if( !Console.HasCartridge() )
          return;
        
        // determine the corresponding card file for current game game
        // otherwise load the corresponding card, if it exists
        string CartridePath = Console.GetCartridgeFileName();
        string MemoryCardPath = GetAutomaticMemoryCardPath( CartridePath );
        
        // ensure that the corresponding card file exists
        if( !FileExists( MemoryCardPath ) )
          GUI_CreateMemoryCard( MemoryCardPath );
        
        // load the card, but don't update latest cards list
        // (leave that for manually used cards)
        Console.LoadMemoryCard( MemoryCardPath );
    }
    
    catch( const exception& e )
    {
        string Message = Texts( TextIDs::Errors_AutoUpdateCard_Label ) + string(e.what());
        DelayedMessageBox( SDL_MESSAGEBOX_ERROR, "Error", Message.c_str() );
    }
}

// -----------------------------------------------------------------------------

void GUI_UnloadCartridge()
{
    try
    {
        Console.UnloadCartridge();
        
        // set window title
        SDL_SetWindowTitle( Video.GetWindow(), "Vircon32: No cartridge" );
        
        // automatic card handling
        if( Emulator.IsCardHandlingAuto() )
          GUI_AutoUpdateMemoryCard();
    }
    
    catch( const exception& e )
    {
        string Message = Texts( TextIDs::Errors_UnloadCartridge_Label ) + string(e.what());
        DelayedMessageBox( SDL_MESSAGEBOX_ERROR, "Error", Message.c_str() );
    }
}

// -----------------------------------------------------------------------------

void GUI_LoadCartridge( string CartridgePath )
{
    try
    {
        if( !CartridgePath.empty() )
        {
            Console.LoadCartridge( CartridgePath );
            Emulator.SetPower( true );
            
            // fix to prevent GUI from drawing
            // on the console's framebuffer
            MouseIsOnWindow = false;
            
            // set window title
            string WindowTitle = string("Vircon32: ") + Console.GetCartridgeTitle();
            SDL_SetWindowTitle( Video.GetWindow(), WindowTitle.c_str() );
            
            // automatic card handling
            if( Emulator.IsCardHandlingAuto() )
              GUI_AutoUpdateMemoryCard();
        }
    }
    
    catch( const exception& e )
    {
        string Message = Texts( TextIDs::Errors_LoadCartridge_Label ) + string(e.what());
        DelayedMessageBox( SDL_MESSAGEBOX_ERROR, "Error", Message.c_str() );
    }
}

// -----------------------------------------------------------------------------

void GUI_ChangeCartridge( string CartridgePath )
{
    try
    {
        if( !CartridgePath.empty() )
        {
            Console.UnloadCartridge();
            Console.LoadCartridge( CartridgePath );
            Emulator.SetPower( true );
            
            // fix to prevent GUI from drawing
            // on the console's framebuffer
            MouseIsOnWindow = false;
            
            // set window title
            string WindowTitle = string("Vircon32: ") + Console.GetCartridgeTitle();
            SDL_SetWindowTitle( Video.GetWindow(), WindowTitle.c_str() );
            
            // automatic card handling
            if( Emulator.IsCardHandlingAuto() )
              GUI_AutoUpdateMemoryCard();
        }
    }
    
    catch( const exception& e )
    {
        string Message = Texts( TextIDs::Errors_ChangeCartridge_Label ) + string(e.what());
        DelayedMessageBox( SDL_MESSAGEBOX_ERROR, "Error", Message.c_str() );
    }
}


// =============================================================================
//      SUPPORT FOR DELAYED FILE GUI ACTIONS
// =============================================================================
// Under Linux there is a risk that performing I/O
// actions related to emulator resource files will
// crash the emulator if they are performed during
// GUI processing. To prevent that, we will instead
// store the requested action to perform it after
// GUI processing has finished for the current frame

enum class DelayedFileActions
{
    None,
    UnloadCartridge,
    LoadCartridge,
    ChangeCartridge,
    CreateMemoryCard,
    UnloadMemoryCard,
    LoadMemoryCard,
    ChangeMemoryCard
};

DelayedFileActions PendingAction = DelayedFileActions::None;
string PendingActionPath = "";


// =============================================================================
//      INDIVIDUAL MENUS IN THE MENU BAR
// =============================================================================


void ProcessMenuConsole()
{
    if( !ImGui::BeginMenu( Texts(TextIDs::Menus_Console) ) )
      return;
    
    if( Emulator.IsPowerOn() )
    {
        if( ImGui::MenuItem( Texts(TextIDs::Console_PowerOff) ) )
          Emulator.SetPower( false );
          
        if( ImGui::MenuItem( Texts(TextIDs::Console_Reset) ) )
        {
            Emulator.Reset();
            ShowEmulatorWindow();
        }
    }
    else
    {
        if( ImGui::MenuItem( Texts(TextIDs::Console_PowerOn) ) )
        {
            Emulator.SetPower( true );
            MouseIsOnWindow = false;
        }
    }
    
    ImGui::Separator();
    
    if( ImGui::MenuItem( Texts(TextIDs::Console_Quit) ) )
      GlobalLoopActive = false;
    
    ImGui::EndMenu();
}

// -----------------------------------------------------------------------------

void ProcessMenuOptions()
{
    if( !ImGui::BeginMenu( Texts(TextIDs::Menus_Options) ) )
      return;
    
    if( ImGui::BeginMenu( Texts(TextIDs::Options_SoundVolume) ) )
    {
        // process volume slider
        int Volume = 100 * Audio.GetOutputVolume();
        ImGui::SetNextItemWidth( 80 * Video.GetRelativeWindowWidth() );
        
        if( ImGui::SliderInt( "##Volume", &Volume, 0, 100 ) )
          Audio.SetOutputVolume( Volume / 100.0 );
        
        // process mute checkbox
        bool Mute = Audio.IsMuted();
        
        if( ImGui::Checkbox( Texts(TextIDs::Options_Mute), &Mute ) )
          Audio.SetMute( Mute );
        
        ImGui::EndMenu();
    }
    
    if( ImGui::BeginMenu( Texts(TextIDs::Options_Language) ) )
    {
        if( ImGui::MenuItem( Texts(TextIDs::Options_English), nullptr, (CurrentLanguage == &LanguageEnglish[0]), true ) )
          SetLanguage( "English" );
          
        if( ImGui::MenuItem( Texts(TextIDs::Options_Spanish), nullptr, (CurrentLanguage == &LanguageSpanish[0]), true ) )
          SetLanguage( "Spanish" );
        
        ImGui::EndMenu();
    }
    
    ImGui::EndMenu();
}

// -----------------------------------------------------------------------------

void ProcessMenuHelp()
{
    if( !ImGui::BeginMenu( Texts(TextIDs::Menus_Help) ) )
      return;
    
    if( ImGui::MenuItem( Texts(TextIDs::Help_QuickGuide) ) )
    {
        DelayedMessageBox
        (
            SDL_MESSAGEBOX_INFORMATION,
            Texts( TextIDs::Dialogs_Guide_Title ),
            Texts( TextIDs::Dialogs_Guide_Label )
        );
    }
    
    if( ImGui::MenuItem( Texts(TextIDs::Help_ShowReadme) ) )
    {
        // unfortunately the way to open a text file is
        // dependent on the underlying operating system
        #if defined(__linux__)
          string ReadmeCommand = "xdg-open \"" + EmulatorFolder + Texts(TextIDs::FileNames_Readme) + "\"";
          system( ReadmeCommand.c_str() );
          
        #elif defined(WINDOWS_OS)
          string ReadmeCommand = "start notepad \"" + EmulatorFolder + Texts(TextIDs::FileNames_Readme) + "\"";
          wstring ReadmeCommandUTF16 = ToUTF16( ReadmeCommand );
          _wsystem( ReadmeCommandUTF16.c_str() );
        
        #elif defined(__APPLE__)
          string ReadmeCommand = "open -a TextEdit \"" + EmulatorFolder + Texts(TextIDs::FileNames_Readme) + "\"";
          system( ReadmeCommand.c_str() );
          
        #else
          #error No information on how to show Readme file in this operating system!

        #endif
    }
    
    if( ImGui::MenuItem( Texts(TextIDs::Help_About) ) )
    {
        DelayedMessageBox
        (
            SDL_MESSAGEBOX_INFORMATION,
            Texts( TextIDs::Dialogs_About_Title ),
            Texts( TextIDs::Dialogs_About_Label )
        );
    }
    
    ImGui::EndMenu();
}

// -----------------------------------------------------------------------------

void ProcessLabelCPU()
{
    ImGui::PushStyleVar(ImGuiStyleVar_Alpha, ImGui::GetStyle().Alpha * 0.5f);
    
    // loads are not applicable if the machine is off
    if( !Emulator.IsPowerOn() )
      ImGui::Text( "%s", Texts(TextIDs::Status_ConsoleOff) );
    
    // not applicable either if the machine is halted
    else if( Console.IsCPUHalted() )
      ImGui::Text( "%s", Texts(TextIDs::Status_CPUHalted) );
    
    // show the maximum load of the last 2 frames
    else
    {
        int CPULoad = Console.GetCPULoad();
        int GPULoad = Console.GetGPULoad();
        ImGui::Text( "CPU %d%%, GPU %d%%", CPULoad, GPULoad );
    }
    
    ImGui::PopStyleVar();
}


// =============================================================================
//      GENERAL GUI RELATED FUNCTIONS
// =============================================================================


// since GUI overlaps the emulator output, we need some
// sensible policy to decide when it is actually needed
bool GUIMustBeDrawn()
{
    return MouseIsOnWindow;
}

// -----------------------------------------------------------------------------

// renders the emulator's framebuffer onto the main program's
// window to make it visible. Since the console implementation
// can change OpenGL's render properties, we need to wrap this
// to ensure the framebuffer is rendered correctly
void ShowEmulatorWindow()
{
    glEnable( GL_BLEND );
    
    // to do the actual drawing on the screen
    // correctly we have to temporarily 
    // override render settings in OpenGL
    GPUColor PreviousMultiplyColor = Video.GetMultiplyColor();
    IOPortValues PreviousBlendingMode = Video.GetBlendingMode();
    Video.SetMultiplyColor( GPUColor{ 255, 255, 255, 255 } );
    Video.SetBlendingMode( IOPortValues::GPUBlendingMode_Alpha );
    
    // if the emulator is on, draw its display on
    // our window; otherwise just show a black screen
    Video.RenderToScreen();
    
    if( Emulator.IsPowerOn() )
      Video.DrawFramebufferOnScreen();
    else
      Video.ClearScreen( GPUColor{ 0, 0, 0, 255 } );
    
    // if GUI is showing, darken the screen
    if( GUIMustBeDrawn() )
      Video.ClearScreen( GPUColor{ 0, 16, 32, 210 } );
    
    // now restore the console's render parameters
    Video.SetMultiplyColor( PreviousMultiplyColor );
    Video.SetBlendingMode( PreviousBlendingMode );
}

// -----------------------------------------------------------------------------

// handles all calls to ImGui to manage and render all of our
// GUI elements onto the main program's window. Since the console
// implementation can change OpenGL's render properties, we need
// to wrap this to ensure the framebuffer is rendered correctly
void RenderGUI()
{
    // delete any previous pending actions
    PendingAction = DelayedFileActions::None;
    PendingActionPath = "";
    
    // remove any emulator blending modes
    IOPortValues PreviousBlendingMode = Video.GetBlendingMode();
    Video.SetBlendingMode( IOPortValues::GPUBlendingMode_Alpha );
    
    // start new frame in imgui
    ImGui_ImplOpenGL3_NewFrame();
    ImGui_ImplSDL2_NewFrame( Video.GetWindow() );
    ImGui::NewFrame();
    
    // show the main menu bar
    if( ImGui::BeginMainMenuBar() )
    {
        // menus
        ProcessMenuConsole();
        ProcessMenuOptions();
        ProcessMenuHelp();
        
        // CPU% label
        ProcessLabelCPU();
        
        ImGui::EndMainMenuBar();
    }
    
    // (2) Render imgui
    if( GUIMustBeDrawn() )
    {
        ImGui::Render();
        ImGui_ImplOpenGL3_RenderDrawData( ImGui::GetDrawData() );
    }
    else ImGui::EndFrame();
    
    // pause the emulator when GUI is used
    if( GUIMustBeDrawn() )
      Emulator.Pause();
    
    else if( Emulator.IsPaused() )
      Emulator.Resume();
    
    // now restore the console's render parameters
    Video.SetBlendingMode( PreviousBlendingMode );
    
    // only after GUI processing is done, we can
    // safely perform any file processing actions
    // that were requested from menu options
    switch( PendingAction )
    {
        // cartridge file actions
        case DelayedFileActions::UnloadCartridge:
            GUI_UnloadCartridge();
            break;
        case DelayedFileActions::LoadCartridge:
            GUI_LoadCartridge( PendingActionPath );
            break;
        case DelayedFileActions::ChangeCartridge:
            GUI_ChangeCartridge( PendingActionPath );
            break;
            
        // memory card file actions
        case DelayedFileActions::CreateMemoryCard:
            GUI_CreateMemoryCard();
            break;
        case DelayedFileActions::UnloadMemoryCard:
            GUI_UnloadMemoryCard();
            break;
        case DelayedFileActions::LoadMemoryCard:
            GUI_LoadMemoryCard( PendingActionPath );
            break;
        case DelayedFileActions::ChangeMemoryCard:
            GUI_ChangeMemoryCard( PendingActionPath );
            break;
        
        // in other cases no actions are performed
        case DelayedFileActions::None: break;
        default: break;
    }
}
