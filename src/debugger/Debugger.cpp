#include "Debugger.hpp"

#include "renderer/Renderer.hpp"
#include "mmaps/NavMesh.hpp"

#include <fstream>
#include <vector>
#include "d3dx9math.h"

namespace Debugger
{
    Debugger * g_debugger = nullptr;

    Debugger::Debugger( VTABLE_TYPE* vtable )
        : m_renderer( vtable )
        , m_memory( GetCurrentProcess() )
        , m_objectMgr( m_memory )
    {
        g_debugger = this;

        Wow::LuaState lua( m_memory );
        lua.Execute( "AccountLoginAccountEdit:SetText('Root')" );
        lua.Execute( "AccountLoginPasswordEdit:SetText('Root')" );
        lua.Execute( "AccountLogin_Login()" );
    }

    Vector2i GetTileCoord( Wow::Location & loc )
    {
        const float     SIZE_OF_GRIDS = 533.0f;
        const float     CENTER_GRID_OFFSET = SIZE_OF_GRIDS / 2.0f;
        const uint32_t  MAX_NUMBER_OF_GRIDS = 64;

        double x_offset = ( double( loc.x ) - CENTER_GRID_OFFSET ) / SIZE_OF_GRIDS;
        double y_offset = ( double( loc.y ) - CENTER_GRID_OFFSET ) / SIZE_OF_GRIDS;

        Vector2i coord;
        coord.x = 63 - int( x_offset + MAX_NUMBER_OF_GRIDS / 2 + 0.5f );
        coord.y = 63 - int( y_offset + MAX_NUMBER_OF_GRIDS / 2 + 0.5f );

        return coord;
    }

    void Debugger::Update()
    {
        if ( !m_objectMgr.IsInWorld() )
            return;

        static LuaFrame s_frame( m_memory );
        if ( !s_frame.IsOpen() )
        {
            s_frame.Open();
        }

        auto player = m_objectMgr.GetLocalPlayer();
        auto camera = m_objectMgr.GetCamera();

        m_navDebugger.Update( player, camera );
    }

    void Debugger::Render()
    {
        m_navDebugger.Render();
    }

    Wow::ObjectManager & Debugger::GetObjectMgr()
    {
        return m_objectMgr;
    }

    extern Debugger * GetDebugger()
    {
        return g_debugger;
    }

    LuaFrame::LuaFrame( ProcessMemory & memory )
        : m_lua( memory )
        , m_isOpen( false )
    {
        m_lua.Execute( R"(
                            DebuggerFrame = CreateFrame("Frame", "DebuggerFrame", UIParent)

                            local DebuggerFrameBackdrop =
                            {
	                            bgFile = "Interface\\DialogFrame\\UI-DialogBox-Background",  
	                            edgeFile = "Interface\\DialogFrame\\UI-DialogBox-Border",
	                            tile = true,
	                            tileSize = 32,
	                            edgeSize = 32,
	                            insets = {left = 11, right = 12, top = 12, bottom = 11}
                            }
                            DebuggerFrame:SetHeight(250)
                            DebuggerFrame:SetWidth(250)
                            DebuggerFrame:SetPoint("LEFT",0,0)
                            DebuggerFrame:SetBackdrop(DebuggerFrameBackdrop)

                            local DebuggerFrameFontString = DebuggerFrame:CreateFontString("DebuggerFrameFontString", "BACKGROUND")
                            DebuggerFrameFontString:SetHeight(200)
                            DebuggerFrameFontString:SetWidth(200)
                            DebuggerFrameFontString:SetPoint("CENTER", "DebuggerFrame", 0, 60)
                            DebuggerFrameFontString:SetFontObject("GameFontNormal")
                            DebuggerFrameFontString:SetText("DEBUGGER FRAME")

                            local DebuggerFrameButton = CreateFrame("Button", "DebuggerFrameButton", DebuggerFrame, "UIPanelButtonTemplate") -- Parent the button to the main frame
                            DebuggerFrameButton:SetPoint("CENTER", 0, -80)
                            DebuggerFrameButton:SetWidth(80)
                            DebuggerFrameButton:SetHeight(22)
                            DebuggerFrameButton:SetText("Close")

                            DebuggerFrameButton:RegisterForClicks("LeftButtonDown")
                            DebuggerFrameButton:SetScript("PostClick", function(self, button,down) DebuggerFrame:Hide() end)
                    )" );
    }

    void LuaFrame::Open()
    {
        m_lua.Execute( R"( DebuggerFrame:Show() )" );

        m_isOpen = true;
    }

    void LuaFrame::Close()
    {

    }
}
