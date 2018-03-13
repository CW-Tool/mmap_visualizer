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
    {
        auto frame = m_lua.CreateFrame( "g_debugger_frame" );

        frame->SetSize( { 200, 250 } );
        frame->SetPosition( { 0, 0 }, Wow::FrameAnchor::LEFT );

        auto header = frame->AddLabel( "g_header_label" );
        header->SetPosition( { 0, 20 }, Wow::FrameAnchor::TOP );
        header->SetSize( { 180, 20 } );
        header->SetText( "SunwellVisualDebugger" );

        auto label1 = frame->AddLabel( "g_navmesh_label" );
        label1->SetPosition( { 5, -20 }, Wow::FrameAnchor::TOPLEFT );
        label1->SetSize( { 160, 20 } );
        label1->SetText( "Render navmesh" );

        m_navMeshBox = frame->AddCheckBox( "g_navmesh_box" );
        m_navMeshBox->SetPosition( { 160, -20 }, Wow::FrameAnchor::TOPLEFT );
        m_navMeshBox->SetSize( { 20, 20 } );
        m_navMeshBox->SetText( "Render navmesh" );
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
        return m_navMeshBox->IsChecked();
    }

}
