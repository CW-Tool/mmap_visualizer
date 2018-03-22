#include "Debugger.hpp"

#include "renderer/Renderer.hpp"
#include "mmaps/NavMesh.hpp"
#include "MinHook.h"

#include <fstream>
#include <vector>
#include "d3dx9math.h"
#include "utils/String.hpp"

namespace Debugger
{
    Debugger * g_debugger = nullptr;

    struct SplineFlags
    {
        uint8_t animId : 8;
        bool done : 1;
        bool falling : 1;
        bool no_spline : 1;
        bool parabolic : 1;
        bool walkmode : 1;
        bool flying : 1;
        bool orientationFixed : 1;
        bool final_point : 1;
        bool final_target : 1;
        bool final_angle : 1;
        bool catmullrom : 1;
        bool cyclic : 1;
        bool enter_cycle : 1;
        bool animation : 1;
        bool frozen : 1;
        bool transportEnter : 1;
        bool transportExit : 1;
        bool unknown7 : 1;
        bool unknown8 : 1;
        bool orientationInversed : 1;
        bool unknown10 : 1;
        bool unknown11 : 1;
        bool unknown12 : 1;
        bool unknown13 : 1;
    };

    enum MonsterMoveType
    {
        MonsterMoveNormal = 0,
        MonsterMoveStop = 1,
        MonsterMoveFacingSpot = 2,
        MonsterMoveFacingTarget = 3,
        MonsterMoveFacingAngle = 4
    };

    Debugger::Debugger( VTABLE_TYPE* vtable )
        : m_renderer( vtable )
        , m_memory( GetCurrentProcess() )
        , m_objectMgr( m_memory )
    {
        m_logFile.open( "F:\\injector.log", std::ios_base::out | std::ios_base::trunc );

        g_debugger = this;

        Wow::LuaState lua( m_memory );
        lua.Execute( "AccountLoginAccountEdit:SetText('Root')" );
        lua.Execute( "AccountLoginPasswordEdit:SetText('Root')" );
        lua.Execute( "AccountLogin_Login()" );

        m_sniffer.AddPacketListener( Opcodes::SMSG_MONSTER_MOVE, [this]( PacketReader & packet )
        {
            auto guid = packet.ReadPackGuid();

            //! flag
            packet.Read< uint8_t >();

            auto start = packet.Read< Vector3f >();

            auto splineId = packet.Read< uint32_t >();

            auto moveType = packet.Read< uint8_t >();
            switch ( moveType )
            {
                case MonsterMoveStop:
                {
                    m_paths.erase( guid );
                    return;
                }
                case MonsterMoveFacingAngle:
                {
                    packet.Read< float >();
                    break;
                }
                case MonsterMoveFacingSpot:
                {
                    packet.Read< Vector3f >();
                    break;
                }
                case MonsterMoveFacingTarget:
                {
                    packet.Read< uint64_t >();
                    break;
                }
                case MonsterMoveNormal:
                {
                    break;
                }
                default:
                    return;
            }

            auto uFlags = packet.Read< uint32_t >();

            auto splineFlags = *reinterpret_cast< SplineFlags * >( &uFlags );
            if ( splineFlags.animation )
            {
                packet.Read< uint8_t >(); //! animId
                packet.Read< int32_t >(); //! start_time
            }

            auto duration = packet.Read< int32_t >();

            if ( splineFlags.parabolic )
            {
                packet.Read< float >();
                packet.Read< int32_t >();
            }

            std::vector< Vector3f > points;
            points.resize( packet.Read< uint32_t >() );

            if ( splineFlags.catmullrom || splineFlags.flying )
            {
                for ( auto idx = 0u; idx < points.size(); ++idx )
                {
                    points[ idx ] = packet.Read<Vector3f>();
                }
            }
            else
            {
                points[ 0 ] = start;

                Vector3f end = packet.Read<Vector3f>();

                Vector3f origin;
                origin.x = ( start.x + end.x ) * 0.5f;
                origin.y = ( start.y + end.y ) * 0.5f;
                origin.z = ( start.z + end.z ) * 0.5f;

                for ( auto idx = 1u; idx < points.size(); ++idx )
                {
                    points[ idx ] = packet.ReadPackXYZ( origin );
                }

                points.push_back( end );
            }

            m_paths[ guid ] = std::make_unique< LineGeometry >( Colors::WhiteAlpha );

            auto & geometry = m_paths[ guid ];
            for ( auto idx = 1; idx < points.size(); ++idx )
            {
                geometry->AddLine( points[ idx -1 ], points[ idx ], Colors::Yellow );
            }
        } );
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

        m_objectMgr.UpdateVisibleObjects();

        if ( m_frame.get() == nullptr )
        {
            m_frame = std::make_unique<DebuggerLuaFrame>( m_memory );
            m_frame->Open();
        }

        auto player = m_objectMgr.GetLocalPlayer();
        auto camera = m_objectMgr.GetCamera();

        if ( m_frame->IsNavMeshVisible() )
        {
            m_navDebugger.Update( player, camera );
        }
    }

    void Debugger::Render()
    {
        if ( m_frame == nullptr )
            return;

        if ( m_frame->IsNavMeshVisible() )
        {
            m_navDebugger.Render();

            m_objectMgr.VisitVisibleObjects( [this]( Wow::ObjectGuid guid )
            {
                auto object = m_objectMgr.GetObject( guid );
                if ( !object )
                    return;

                auto type = object->GetField< uint32_t >( Wow::OBJECT_FIELD_TYPE );
                if ( type & Wow::OBJECT_TYPE_GAMEOBJECT )
                {
                    auto displayId = object->GetField< uint32_t >( Wow::GAMEOBJECT_DISPLAYID );
                    m_navDebugger.RenderModel( displayId, object->GetPositon(), object->GetFacing() );
                }
            } );
        }

        if ( m_frame->IsPathRenderingEnabled() )
        {
            m_objectMgr.VisitVisibleObjects( [this]( Wow::ObjectGuid guid )
            {
                auto it = m_paths.find( guid );
                if ( it == m_paths.end() )
                    return;

                auto & geometry = it->second;

                Wow::Camera camera = GetDebugger()->GetObjectMgr().GetCamera();
                Transform transform = Transform::FromCamera( camera );

                GetRenderer()->RenderGeometry( *geometry, &transform );
            } );
        }
    }

    void Debugger::Log( const std::string& msg )
    {
        std::string buffer = msg;
        buffer.append( "\n" );

        m_logFile.write( buffer.c_str(), buffer.size() );
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

        {
            auto label1 = frame->AddLabel( "g_navmesh_label" );
            label1->SetPosition( { 5, -20 }, Wow::FrameAnchor::TOPLEFT );
            label1->SetSize( { 160, 20 } );
            label1->SetText( "Render navmesh" );

            m_navMeshBox = frame->AddCheckBox( "g_navmesh_box" );
            m_navMeshBox->SetPosition( { 160, -20 }, Wow::FrameAnchor::TOPLEFT );
            m_navMeshBox->SetSize( { 20, 20 } );
        }

        {
            auto label2 = frame->AddLabel( "g_paths_label" );
            label2->SetPosition( { 5, -40 }, Wow::FrameAnchor::TOPLEFT );
            label2->SetSize( { 160, 20 } );
            label2->SetText( "Render paths" );

            m_pathRenderingBox = frame->AddCheckBox( "g_path_box" );
            m_pathRenderingBox->SetPosition( { 160, -40 }, Wow::FrameAnchor::TOPLEFT );
            m_pathRenderingBox->SetSize( { 20, 20 } );
        }
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

    bool DebuggerLuaFrame::IsPathRenderingEnabled()
    {
        return m_pathRenderingBox->IsChecked();
    }
}
