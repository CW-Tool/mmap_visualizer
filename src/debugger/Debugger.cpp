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

        static DebuggerLuaFrame s_frame( m_memory );
        if ( !s_frame.IsOpen() )
        {
            s_frame.Open();
        }

        auto player = m_objectMgr.GetLocalPlayer();
        auto camera = m_objectMgr.GetCamera();

        m_navDebugger.SetEnabled( s_frame.IsNavMeshVisible() );
        if ( m_navDebugger.IsEnabled() )
        {
            m_navDebugger.Update( player, camera );
        }
    }

    void Debugger::Render()
    {
        if ( m_navDebugger.IsEnabled() )
        {
            m_navDebugger.Render();
        }
    }

    void Debugger::RegisterLua( Wow::LuaState & state )
    {
        //luaL_newmetatable( L, "Lua.MyClass" );
        //luaL_register( L, 0, gDestroyMyClassFuncs );
        //luaL_register( L, 0, gMyClassFuncs );
        //lua_pushvalue( L, -1 );
        //lua_setfield( L, -2, "__index" );

        // Register the base class for instances of Sprite
        //luaL_register( L, "MyClass", gSpriteFuncs );
    }

    Wow::ObjectManager & Debugger::GetObjectMgr()
    {
        return m_objectMgr;
    }

    extern Debugger * GetDebugger()
    {
        return g_debugger;
    }

    DebuggerLuaFrame::DebuggerLuaFrame( ProcessMemory & memory )
        : m_lua( memory )
        , m_isOpen( false )
    {
        m_lua.Execute( R"(
                            g_debugger_frame = CreateFrame("Frame", "g_debugger_frame", UIParent)
                            g_debugger_frame:SetHeight(250)
                            g_debugger_frame:SetWidth(250)
                            g_debugger_frame:SetPoint("LEFT",0,0)
                            g_debugger_frame:SetBackdrop(
                            {
	                            bgFile = "Interface\\DialogFrame\\UI-DialogBox-Background",  
	                            edgeFile = "Interface\\DialogFrame\\UI-DialogBox-Border",
	                            tile = true,
	                            tileSize = 32,
	                            edgeSize = 32,
	                            insets = {left = 11, right = 12, top = 12, bottom = 11}
                            })

                            local header_label = g_debugger_frame:CreateFontString("g_header_label", "BACKGROUND")
                            header_label:SetHeight(20)
                            header_label:SetWidth(200)
                            header_label:SetPoint("TOP", "g_debugger_frame", 0, 20)
                            header_label:SetFontObject("GameFontNormal")
                            header_label:SetText("SunwellVisualDebugger")

                            local check_box_label = g_debugger_frame:CreateFontString("g_navmesh_label", "BACKGROUND")
                            check_box_label:SetHeight(20)
                            check_box_label:SetWidth(200)
                            check_box_label:SetPoint("TOPLEFT", "g_debugger_frame", -20, -20)
                            check_box_label:SetFontObject("GameFontNormal")
                            check_box_label:SetText("Render navmesh")

                            g_debugger_frame.render_navmesh = false

                            local check_box = CreateFrame("CheckButton", "g_navmesh_box", g_debugger_frame, "ChatConfigCheckButtonTemplate")
                            check_box:SetPoint("TOPLEFT", 200, -20)
                            check_box:SetWidth(20)
                            check_box:SetHeight(20)
                            check_box:SetScript("OnClick", function() g_debugger_frame.render_navmesh = not g_debugger_frame.render_navmesh end );
                    )" );
    }

    void DebuggerLuaFrame::Open()
    {
        //m_lua.Execute( R"( g_debugger_frame:Show() )" );

        m_isOpen = true;
    }

    void DebuggerLuaFrame::Close()
    {

    }

    bool DebuggerLuaFrame::IsNavMeshVisible()
    {
        auto result = m_lua.Execute( R"( return g_debugger_frame.render_navmesh == true )" );
        if ( !result || result->empty() )
            return false;

        return std::get< bool >( result->front() );
    }

}
