#pragma once
#include "imgui.h"


namespace ImGameUI
{

enum Sound
{
    Sound_ItemEntered,
    Sound_ButtonClicked,
    Sound_SliderChanged,
    Sound_CheckboxTicked,
    Sound_ComboboxSelected,
    SoundCount
};


struct SoundPlayer
{
    virtual void play( Sound sound ) = 0;
};


class GameUI
{
public:

    struct Config
    {
        const char*         m_titleFont = nullptr;
        float               m_titleFontSize = 0;
        const char*         m_itemFont = nullptr;
        float               m_itemFontSize = 0;
        const char*         m_smallItemFont = nullptr;
        float               m_smallItemFontSize = 0;

        SoundPlayer*        m_soundPlayer = nullptr;

        struct Style
        {
            int                 m_menuWidth = 1000;
            int                 m_menuHeight = 600;
            ImVec4              m_colourText = ImVec4( 1.0f, 1.0f, 0.0f, 1.0f );
            ImVec4              m_colourDisabledText = ImVec4( 0.5f, 0.5f, 0.5f, 1.0f );
            ImVec4              m_colourUnTexturedWindow = ImVec4( 0.17f, 0.17f, 0.9f, 0.5f );
            ImVec4              m_colourTexturedWindow = ImVec4( 1.0f, 1.0f, 1.0f, 0.3f );
            ImVec4              m_colourTitleBar = ImVec4( 0.0f, 0.0f, 0.2f, 0.7f );
        } m_style = {};
    };

    GameUI();
    virtual ~GameUI();

    // Init the system once. This will load the fonts etc..
    void init( const Config& config );

    // Start the immediate mode rendering of the UI. This renders the menu screen backdrop.
    void begin( int screenWidth, int screenHeight, void* textureHandle );

    // Finish the immediate mode rendering of the UI. After this, you will still need to call ImGui::Render() and ImGui_Impl*_RenderDrawData().
    void end();

    enum Flags
    {
        Flags_SmallItems    = 1 << 0,       // Use the small font and use small widgets.
        Flags_ScrollWindow  = 1 << 1        // If there are lots of items on UI, then you can render them in a scrollable sub window.
    };

    // Draws the title bar over the backdrop
    void drawTitleBar( const char* text );

    // Begins rendering a group of items below the title bar
    void beginItems( unsigned int flags );
    void endItems();

    bool checkBox( const char* name, bool* value );
    bool slider( const char* name, float* value );
    bool comboBox( const char* name, int numOptions, const char* options[], int* index );
    bool button( const char* label );

private:

    void applyGameUIStyle( float scale );
    float screenX( int x ) const;
    float screenY( int y ) const;
    ImVec2 screenXY( int x, int y ) const;
    void playSound( Sound sound );

    enum FontType
    {
        Font_Title,
        Font_Item,
        Font_ItemSmall,
        FontTypeCount
    };

    ImFont*         m_fonts[ FontTypeCount ] = {};
    float           m_aspectCorrection = 1.0f;
    float           m_screenWidth = 0.0f;
    float           m_screenHeight = 0.0f;
    unsigned int    m_currentFlags = 0;
    SoundPlayer*    m_soundPlayer = nullptr;
    Config::Style   m_style = {};
    ImGuiID         m_currentHoveredID = 0;         // Keep track of which item is hovered over for sound activation
    ImGuiID         m_currentMenuID = 0;            // Keep track of the last menu to see if the menu has changed
    bool            m_newMenuThisFrame = false;     // Flag to say the menu if new this frame
};

}