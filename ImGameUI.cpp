#include "ImGameUI.h"
#include <string.h>
#include "imgui_internal.h"


namespace ImGameUI
{

const float gUnitsX = 1920.0f;
const float gUnitsY = 1080.0f;
//const int gMenuWidth = 1000;
//const int gMenuHeight = 600;


GameUI::GameUI()
{

}


GameUI::~GameUI()
{

}


void GameUI::init( const Config& config )
{
    ImGuiIO& io = ImGui::GetIO();
    m_fonts[ Font_Title ] = io.Fonts->AddFontFromFileTTF( config.m_titleFont, config.m_titleFontSize );
    m_fonts[ Font_Item ] = io.Fonts->AddFontFromFileTTF( config.m_itemFont, config.m_itemFontSize );
    m_fonts[ Font_ItemSmall ] = io.Fonts->AddFontFromFileTTF( config.m_smallItemFont, config.m_smallItemFontSize );

    m_soundPlayer = config.m_soundPlayer;

    m_style = config.m_style;
}


void GameUI::begin( int screenWidth, int screenHeight, void* textureHandle )
{
    m_screenWidth = (float)screenWidth;
    m_screenHeight = (float)screenHeight;
    m_aspectCorrection = (gUnitsX / gUnitsY) / (m_screenWidth / m_screenHeight);

    float scale = m_screenHeight / gUnitsY;

    m_newMenuThisFrame = false;

    applyGameUIStyle( scale );

    ImGui::NewFrame();

    const auto windowFlags =
        ImGuiWindowFlags_NoDocking             |
        ImGuiWindowFlags_NoTitleBar            |
        ImGuiWindowFlags_NoCollapse            |
        ImGuiWindowFlags_NoResize              |
        ImGuiWindowFlags_NoMove                |
        ImGuiWindowFlags_NoScrollbar           |
        ImGuiWindowFlags_NoBringToFrontOnFocus |
        ImGuiWindowFlags_NoNavFocus;

    ImVec2 pos;
    pos.x = 0.5f * ( m_screenWidth - screenX( m_style.m_menuWidth ) );
    pos.y = 0.5f * ( m_screenHeight - screenY( m_style.m_menuHeight ) );

    const ImVec2 windowSize = screenXY( m_style.m_menuWidth, m_style.m_menuHeight );

    ImGui::SetNextWindowPos( pos );
    ImGui::SetNextWindowSize( windowSize );

    ImGui::Begin( "##main_window", nullptr, windowFlags );

    if ( m_style.m_colourTexturedWindow.w > 0.0f && textureHandle )
    {
        const ImVec2 cursorPos = ImGui::GetCursorScreenPos();
        const ImRect windowRect( cursorPos.x, cursorPos.y, cursorPos.x + windowSize.x, cursorPos.y + windowSize.y );

        ImGui::GetWindowDrawList()->AddImage( textureHandle, windowRect.Min, windowRect.Max, ImVec2( 0.0f, 0.0f ), ImVec2( 1.0f, 1.0f ), ImGui::GetColorU32( m_style.m_colourTexturedWindow ) );
    }
}


void GameUI::end()
{
    if ( !ImGui::IsAnyItemHovered() )
    {
        m_currentHoveredID = 0;
    }

    ImGui::End();
    ImGui::EndFrame();
}


float GameUI::screenX( int x ) const
{
    return m_aspectCorrection * m_screenWidth * ( (float)x / gUnitsX );
}


float GameUI::screenY( int y ) const
{
    return m_screenHeight * ( (float)y / gUnitsY );
}


ImVec2 GameUI::screenXY( int x, int y ) const
{
    return ImVec2( screenX( x ), screenY( y ) );
}


void GameUI::beginItems( unsigned int flags )
{
    m_currentFlags = flags;
    ImGui::PushFont( flags & Flags_SmallItems ? m_fonts[ Font_ItemSmall ] : m_fonts[ Font_Item ] );

    if ( flags & Flags_ScrollWindow )
    {
        ImGui::SetCursorPosX( screenX( 20 ) );
        ImGui::PushStyleColor( ImGuiCol_ChildBg, ImVec4( 0, 0, 0, 0.1f ) );
        ImGui::BeginChild( "##ContentRegion", screenXY( m_style.m_menuWidth - 40, m_style.m_menuHeight - 240 ) );
    }
}


void GameUI::endItems()
{
    if ( m_currentFlags & Flags_ScrollWindow )
    {
        ImGui::EndChild();
        ImGui::PopStyleColor();
    }
    ImGui::PopFont();
    m_currentFlags = 0;
}


bool GameUI::checkBox( const char* name, bool* value )
{
    char itemName[ 64 ] = { "##" };
    strcat_s( itemName, _countof( itemName ), name );

    float contentRegionWidth = ImGui::GetContentRegionAvail().x;

    ImGui::SetCursorPosX( screenX( 20 ) );
    ImGui::Text( name ); ImGui::SameLine();

    ImGui::SetCursorPosX( contentRegionWidth - screenX( (int)( 140 ) ) );
    bool valueChanged = ImGui::Checkbox( itemName, value );
    if ( valueChanged )
    {
        playSound( Sound_CheckboxTicked );
    }
    return valueChanged;
}


bool GameUI::slider( const char* name, float* value )
{
    char itemName[ 64 ] = { "##" };
    strcat_s( itemName, _countof( itemName ), name );

    ImGui::SetCursorPosX( screenX( 20 ) );
    ImGui::Text( name ); ImGui::SameLine();
    ImGui::SetCursorPosX( screenX( (int)( m_style.m_menuWidth / 3.0f ) ) );
    bool valueChanged = ImGui::SliderFloat( itemName, value, 0.0f, 1.0f, "%.1f" );
    if ( valueChanged )
    {
        playSound( Sound_SliderChanged );
    }
    return valueChanged;
}


bool GameUI::comboBox( const char* name, int numOptions, const char* options[], int* index )
{
    char itemName[ 64 ] = { "##" };
    strcat_s( itemName, _countof( itemName ), name );

    float contentRegionWidth = ImGui::GetContentRegionAvail().x;

    ImGui::SetCursorPosX( screenX( 20 ) );
    ImGui::Text( name );
    ImGui::SameLine();

    ImGui::SetCursorPosX( contentRegionWidth - screenX( 520 ) );
    ImGui::PushItemWidth( screenX( 500 ) );
    assert( *index >= 0 && *index < numOptions );

    bool valueChanged = false;

    if ( ImGui::BeginCombo( itemName, options[ *index ], 0 ) )
    {
        for ( int i = 0; i < numOptions; i++ )
        {
            if ( ImGui::Selectable( options[ i ] ) )
            {
                *index = i;
                valueChanged = true;
            }
        }
        ImGui::EndCombo();
    }
    ImGui::PopItemWidth();

    if ( valueChanged )
    {
        playSound( Sound_ComboboxSelected );
    }

    return valueChanged;
}


void GameUI::drawTitleBar( const char* text )
{
    const ImGuiStyle& style = ImGui::GetStyle();

    ImGui::PushFont( m_fonts[ Font_Title ] );

    const ImVec2 itemSize( screenX( m_style.m_menuWidth ), ImGui::CalcTextSize( text ).y + style.FramePadding.y * 2.0f );

    const ImVec2 cursorPos = ImGui::GetWindowPos();
    const ImRect buttonRect( cursorPos.x, cursorPos.y, cursorPos.x + itemSize.x, cursorPos.y + itemSize.y );

    if ( m_style.m_colourTitleBar.w > 0.0f )
    {
        ImGui::GetWindowDrawList()->AddRectFilled( buttonRect.Min, buttonRect.Max, ImGui::GetColorU32( m_style.m_colourTitleBar ) );
        // ImGui::GetWindowDrawList()->AddImage( m_menuTexture->getHandle(), buttonRect.Min, buttonRect.Max, ImVec2( 0.0f, 0.0f ), ImVec2( 1.0f, 0.2f ), IM_COL32( 255, 255, 255, 100 ) );
    }

    ImGui::SetCursorPosX( (ImGui::GetWindowWidth() - ImGui::CalcTextSize(text).x) / 2.f);
    ImGui::SetCursorPosY( ImGui::GetCursorPosY() + style.FramePadding.y );
    ImGui::Text( text );

    ImGuiID id = ImGui::GetID( text );
    if ( m_currentMenuID != id )
    {
        m_currentMenuID = id;
        m_newMenuThisFrame = true;
    }

    ImGui::PopFont();

    ImGui::SetCursorPosY( ImGui::GetCursorPosY() + style.FramePadding.y );
}


bool GameUI::button( const char* label )
{
    ImGuiStyle& style = ImGui::GetStyle();

    float avail = ImGui::GetContentRegionAvail().x;

    float itemWidth = screenX( m_style.m_menuWidth - 100 );

    float off = (avail - itemWidth) * 0.5f;
    if ( off > 0.0f )
        ImGui::SetCursorPosX(ImGui::GetCursorPosX() + off);

    bool valueChanged = ImGui::Button( label, ImVec2( itemWidth, 0.0f ) );
    if ( valueChanged )
    {
        playSound( Sound_ButtonClicked );
    }

    ImGuiID id = ImGui::GetID( label );
    if ( ImGui::IsItemHovered() && m_currentHoveredID != id )
    {
        m_currentHoveredID = id;
        if ( !valueChanged && !m_newMenuThisFrame )
        {
            playSound( Sound_ItemEntered );
        }
    }

    return valueChanged;
}


void GameUI::applyGameUIStyle( float scale )
{
    ImGuiStyle& style = ImGui::GetStyle();

    style.Alpha = 1.0;
    style.DisabledAlpha = 0.6000000238418579;
    style.WindowPadding = ImVec2(0.0, 0.0);
    style.WindowRounding = 0.0;
    style.WindowBorderSize = 0.0;
    style.WindowMinSize = ImVec2(32.0, 32.0);
    style.WindowTitleAlign = ImVec2(0.0, 0.0);
    style.WindowMenuButtonPosition = ImGuiDir_Right;
    style.ChildRounding = 0.0;
    style.ChildBorderSize = 0.0;
    style.PopupRounding = 0.0;
    style.PopupBorderSize = 0.0;
    style.FramePadding = ImVec2(4.0, 4.0);      // the space around boxes for text
    style.FrameRounding = 0.0;
    style.FrameBorderSize = 0.0;
    style.ItemSpacing = ImVec2(8.0, 4.0);
    style.ItemInnerSpacing = ImVec2(4.0, 4.0);
    style.CellPadding = ImVec2(4.0, 2.0);
    style.IndentSpacing = 21.0;
    style.ColumnsMinSpacing = 6.0;
    style.ScrollbarSize = 10.0;
    style.ScrollbarRounding = 1.0;
    style.GrabMinSize = 7.0;
    style.GrabRounding = 0.0;
    style.TabRounding = 0.0;
    style.TabBorderSize = 1.0;
    style.TabMinWidthForCloseButton = 0.0;
    style.ColorButtonPosition = ImGuiDir_Right;
    style.ButtonTextAlign = ImVec2(0.5, 0.5);
    style.SelectableTextAlign = ImVec2(0.0, 0.0);

#pragma warning(push)
#pragma warning(disable: 4305 )

    style.Colors[ImGuiCol_Text] = m_style.m_colourText;
    style.Colors[ImGuiCol_TextDisabled] = m_style.m_colourDisabledText;
    style.Colors[ImGuiCol_WindowBg] = m_style.m_colourUnTexturedWindow;
    style.Colors[ImGuiCol_ChildBg] = ImVec4(0.2784313857555389, 0.2784313857555389, 0.2784313857555389, 1.0);
    style.Colors[ImGuiCol_PopupBg] = ImVec4(0.3098039329051971, 0.3098039329051971, 0.3098039329051971, 1.0);
    style.Colors[ImGuiCol_Border] = ImVec4(0.2627451121807098, 0.2627451121807098, 0.2627451121807098, 1.0);
    style.Colors[ImGuiCol_BorderShadow] = ImVec4(0.0, 0.0, 0.0, 0.0);
    style.Colors[ImGuiCol_FrameBg] = ImVec4(0.1568627506494522, 0.1568627506494522, 0.1568627506494522, 1.0);
    style.Colors[ImGuiCol_FrameBgHovered] = ImVec4(0.2000000029802322, 0.2000000029802322, 0.2000000029802322, 1.0);
    style.Colors[ImGuiCol_FrameBgActive] = ImVec4(0.2784313857555389, 0.2784313857555389, 0.2784313857555389, 1.0);
    style.Colors[ImGuiCol_TitleBg] = ImVec4(0.1450980454683304, 0.1450980454683304, 0.1450980454683304, 1.0);
    style.Colors[ImGuiCol_TitleBgActive] = ImVec4(0.1450980454683304, 0.1450980454683304, 0.1450980454683304, 1.0);
    style.Colors[ImGuiCol_TitleBgCollapsed] = ImVec4(0.1450980454683304, 0.1450980454683304, 0.1450980454683304, 1.0);
    style.Colors[ImGuiCol_MenuBarBg] = ImVec4(0.1921568661928177, 0.1921568661928177, 0.1921568661928177, 1.0);
    style.Colors[ImGuiCol_ScrollbarBg] = ImVec4(0.1568627506494522, 0.1568627506494522, 0.1568627506494522, 1.0);
    style.Colors[ImGuiCol_ScrollbarGrab] = ImVec4(0.2745098173618317, 0.2745098173618317, 0.2745098173618317, 1.0);
    style.Colors[ImGuiCol_ScrollbarGrabHovered] = ImVec4(0.2980392277240753, 0.2980392277240753, 0.2980392277240753, 1.0);
    style.Colors[ImGuiCol_ScrollbarGrabActive] = ImVec4(1.0, 0.3882353007793427, 0.0, 1.0);
    style.Colors[ImGuiCol_CheckMark] = ImVec4(1.0, 1.0, 1.0, 1.0);
    style.Colors[ImGuiCol_SliderGrab] = ImVec4(0.3882353007793427, 0.3882353007793427, 0.3882353007793427, 1.0);
    style.Colors[ImGuiCol_SliderGrabActive] = ImVec4(1.0, 0.3882353007793427, 0.0, 1.0);
    style.Colors[ImGuiCol_Button] = ImVec4(1.0, 1.0, 1.0, 0.0);
    style.Colors[ImGuiCol_ButtonHovered] = ImVec4(1.0, 1.0, 1.0, 0.1560000032186508);
    style.Colors[ImGuiCol_ButtonActive] = ImVec4(1.0, 1.0, 1.0, 0.3910000026226044);
    style.Colors[ImGuiCol_Header] = ImVec4(0.3098039329051971, 0.3098039329051971, 0.3098039329051971, 1.0);
    style.Colors[ImGuiCol_HeaderHovered] = ImVec4(0.4666666686534882, 0.4666666686534882, 0.4666666686534882, 1.0);
    style.Colors[ImGuiCol_HeaderActive] = ImVec4(0.4666666686534882, 0.4666666686534882, 0.4666666686534882, 1.0);
    style.Colors[ImGuiCol_Separator] = ImVec4(0.2627451121807098, 0.2627451121807098, 0.2627451121807098, 1.0);
    style.Colors[ImGuiCol_SeparatorHovered] = ImVec4(0.3882353007793427, 0.3882353007793427, 0.3882353007793427, 1.0);
    style.Colors[ImGuiCol_SeparatorActive] = ImVec4(1.0, 0.3882353007793427, 0.0, 1.0);
    style.Colors[ImGuiCol_ResizeGrip] = ImVec4(1.0, 1.0, 1.0, 0.25);
    style.Colors[ImGuiCol_ResizeGripHovered] = ImVec4(1.0, 1.0, 1.0, 0.6700000166893005);
    style.Colors[ImGuiCol_ResizeGripActive] = ImVec4(1.0, 0.3882353007793427, 0.0, 1.0);
    style.Colors[ImGuiCol_Tab] = ImVec4(0.09411764889955521, 0.09411764889955521, 0.09411764889955521, 1.0);
    style.Colors[ImGuiCol_TabHovered] = ImVec4(0.3490196168422699, 0.3490196168422699, 0.3490196168422699, 1.0);
    style.Colors[ImGuiCol_TabActive] = ImVec4(0.1921568661928177, 0.1921568661928177, 0.1921568661928177, 1.0);
    style.Colors[ImGuiCol_TabUnfocused] = ImVec4(0.09411764889955521, 0.09411764889955521, 0.09411764889955521, 1.0);
    style.Colors[ImGuiCol_TabUnfocusedActive] = ImVec4(0.1921568661928177, 0.1921568661928177, 0.1921568661928177, 1.0);
    style.Colors[ImGuiCol_PlotLines] = ImVec4(0.4666666686534882, 0.4666666686534882, 0.4666666686534882, 1.0);
    style.Colors[ImGuiCol_PlotLinesHovered] = ImVec4(1.0, 0.3882353007793427, 0.0, 1.0);
    style.Colors[ImGuiCol_PlotHistogram] = ImVec4(0.5843137502670288, 0.5843137502670288, 0.5843137502670288, 1.0);
    style.Colors[ImGuiCol_PlotHistogramHovered] = ImVec4(1.0, 0.3882353007793427, 0.0, 1.0);
    style.Colors[ImGuiCol_TableHeaderBg] = ImVec4(0.1882352977991104, 0.1882352977991104, 0.2000000029802322, 1.0);
    style.Colors[ImGuiCol_TableBorderStrong] = ImVec4(0.3098039329051971, 0.3098039329051971, 0.3490196168422699, 1.0);
    style.Colors[ImGuiCol_TableBorderLight] = ImVec4(0.2274509817361832, 0.2274509817361832, 0.2470588237047195, 1.0);
    style.Colors[ImGuiCol_TableRowBg] = ImVec4(0.0, 0.0, 0.0, 0.0);
    style.Colors[ImGuiCol_TableRowBgAlt] = ImVec4(1.0, 1.0, 1.0, 0.05999999865889549);
    style.Colors[ImGuiCol_TextSelectedBg] = ImVec4(1.0, 1.0, 1.0, 0.1560000032186508);
    style.Colors[ImGuiCol_DragDropTarget] = ImVec4(1.0, 0.3882353007793427, 0.0, 1.0);
    style.Colors[ImGuiCol_NavHighlight] = ImVec4(1.0, 0.3882353007793427, 0.0, 1.0);
    style.Colors[ImGuiCol_NavWindowingHighlight] = ImVec4(1.0, 0.3882353007793427, 0.0, 1.0);
    style.Colors[ImGuiCol_NavWindowingDimBg] = ImVec4(0.0, 0.0, 0.0, 0.5860000252723694);
    style.Colors[ImGuiCol_ModalWindowDimBg] = ImVec4(0.0, 0.0, 0.0, 0.5860000252723694);
#pragma warning(pop)
    style.ScaleAllSizes( scale * 5.0f );

    m_fonts[ Font_Title ]->Scale = scale * 0.5f;
    m_fonts[ Font_Item ]->Scale = scale * 0.5f;
    m_fonts[ Font_ItemSmall ]->Scale = scale * 0.5f;
}


void GameUI::playSound( Sound sound )
{
    if ( m_soundPlayer )
    {
        m_soundPlayer->play( sound );
    }
}

};