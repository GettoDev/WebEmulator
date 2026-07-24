// *****************************************************************************
    // start include guard
    #ifndef GUI_HPP
    #define GUI_HPP
    
    // include C/C++ headers
    #include <string>           // [ C++ STL ] Strings
    #include <cstdint>          // [ ANSI C ] Standard integer types
// *****************************************************************************


// =============================================================================
//      DELAYED MESSAGE BOX FUNCTIONS
// =============================================================================


void DelayedMessageBox( uint32_t flags, const char *Title, const char *Message );
void ShowDelayedMessageBox();
void CancelDelayedMessageBox();


// =============================================================================
//      ADDITIONAL GUI FUNCTIONS
// =============================================================================


void SetWindowZoom1X();
std::string GetAutomaticMemoryCardPath( const std::string& CartridgePath );


// =============================================================================
//      ENCAPSULATED GUI FUNCTIONS
// =============================================================================


void GUI_CreateMemoryCard( std::string MemoryCardPath = "" );
void GUI_UnloadMemoryCard();
void GUI_LoadMemoryCard( std::string MemoryCardPath = "" );
void GUI_ChangeMemoryCard( std::string MemoryCardPath = "" );
void GUI_AutoUpdateMemoryCard();
void GUI_UnloadCartridge();
void GUI_LoadCartridge( std::string CartridgePath = "" );
void GUI_ChangeCartridge( std::string CartridgePath = "" );


// =============================================================================
//      GENERAL GUI RELATED FUNCTIONS
// =============================================================================


bool GUIMustBeDrawn();
void RenderGUI();
void ShowEmulatorWindow();


// *****************************************************************************
    // end include guard
    #endif
// *****************************************************************************
