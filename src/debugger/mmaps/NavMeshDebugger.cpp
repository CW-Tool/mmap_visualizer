#include "NavMeshDebugger.hpp"

#include "Debugger.hpp"
#include "renderer/Renderer.hpp"

#include "math/Vector.hpp"
#include "utils/String.hpp"

#include <fstream>
#include "DetourNavMesh.h"
#include "NavMesh.hpp"
#include "DetourDebugDraw.h"

namespace Debugger
{
    inline int dtAlign4( int x )
    {
        return ( x + 3 ) & ~3;
    }

    NavMeshDebugger::NavMeshDebugger()
    {
    }

    template< class T >
    void hash_combine( uint32_t & seed, T value )
    {
        seed ^= std::hash<T>()( value ) + 0x9e3779b9 + ( seed << 6 ) + ( seed >> 2 );
    }

    size_t GetMapTileHash( uint32_t mapId, const Vector2i &coord )
    {
        size_t hash = 0u;
        hash_combine( hash, mapId );
        hash_combine( hash, coord.x );
        hash_combine( hash, coord.y );

        return hash;
    }

    void NavMeshDebugger::Render()
    {
        auto player = GetDebugger()->GetObjectMgr().GetLocalPlayer();
        uint32_t mapId = player.GetMapId();

        Wow::Location loc = player.GetLocation();
        Vector2i tile = GetTileCoord( loc );

        auto tileHash = GetMapTileHash( mapId, tile );
        auto it = m_mapGeometry.find( tileHash );
        if ( it != m_mapGeometry.end() )
        {
            Wow::Camera camera = GetDebugger()->GetObjectMgr().GetCamera();
            Transform transform = Transform::FromCamera( camera );

            auto & data = it->second;

            auto renderer = GetRenderer();
            for ( auto && geom : data.m_geometry )
            {
                renderer->RenderGeometry( *geom, &transform );
            }
        }
    }

    void NavMeshDebugger::RenderModel( uint32_t modelId, const Wow::Location & pos, float rot )
    {
        if ( !LoadModelNavMesh( modelId ) )
            return;

        Wow::Camera camera = GetDebugger()->GetObjectMgr().GetCamera();

        Transform transform = Transform::FromCamera( camera );

        D3DXQUATERNION quat;
        D3DXQuaternionRotationYawPitchRoll( D3DXQuaternionIdentity( &quat ), 0.0f, 0.0f, rot );

        D3DXVECTOR3 trans( pos.x, pos.y, pos.z );
        D3DXMatrixTransformation( &transform.world, nullptr, nullptr, nullptr, nullptr, &quat, &trans );

        auto & data = m_modelGeometry[ modelId ];

        auto renderer = GetRenderer();
        for ( auto && geom : data.m_geometry )
        {
            renderer->RenderGeometry( *geom, &transform );
        }
    }

    void NavMeshDebugger::Update( Wow::Player & player, Wow::Camera & camera )
    {
        uint32_t mapId = player.GetMapId();

        Wow::Location loc = player.GetLocation();
        Vector2i tile = GetTileCoord( loc );

        LoadMapNavMesh( mapId, tile );
    }

    void NavMeshDebugger::Clear()
    {
        m_mapGeometry.clear();
        m_modelGeometry.clear();
    }

    void LoadNavMeshTile( std::ifstream &file, dtNavMesh & navMesh, std::vector<uint8_t> & data, DebugDetourDraw & geometry )
    {
        NavTileHeader tileHeader;
        file.read( reinterpret_cast< char * >( &tileHeader ), sizeof( NavTileHeader ) );

        data.resize( tileHeader.size );

        file.read( reinterpret_cast< char * >( &data[ 0 ] ), tileHeader.size );

        dtMeshHeader* header = reinterpret_cast< dtMeshHeader* >( &data[ 0 ] );
        dtTileRef tileRef = 0;

        navMesh.addTile( &data[ 0 ], tileHeader.size, 0, 0, &tileRef );

        duDebugDrawNavMesh( &geometry, navMesh, 0xFF );
    }

    bool NavMeshDebugger::LoadMapNavMesh( uint32_t mapId, const Vector2i & coord )
    {
        if ( m_mapGeometry.find( GetMapTileHash( mapId, coord ) ) != m_mapGeometry.end() )
            return true;

        std::ifstream file1( Format( "F:\\Sunwell\\emu_data\\mmaps\\%03u.mmap", mapId ), std::ios::in | std::ios::binary );
        if ( !file1.is_open() )
            return false;

        dtNavMeshParams params;
        file1.read( reinterpret_cast< char * >( &params ), sizeof( dtNavMeshParams ) );

        dtNavMesh navMesh;
        if ( navMesh.init( &params ) != DT_SUCCESS )
            return false;

        std::ifstream file2( Format( "F:\\Sunwell\\emu_data\\mmaps\\%03u%02u%02u.mmtile", mapId, coord.x, coord.y ), std::ios::in | std::ios::binary );
        if ( !file2.is_open() )
            return false;

        auto & debugDraw = m_mapGeometry[ GetMapTileHash( mapId, coord ) ];

        std::vector< uint8_t > data;
        LoadNavMeshTile( file2, navMesh, data, debugDraw );

        return true;
    }

    bool NavMeshDebugger::LoadModelNavMesh( uint32_t modelId )
    {
        if ( m_modelGeometry.find( modelId ) != m_modelGeometry.end() )
            return true;

        std::ifstream file1( Format( "F:\\Sunwell\\emu_data\\mmaps\\models\\%03u.mmap", modelId ), std::ios::in | std::ios::binary );
        if ( !file1.is_open() )
            return false;

        dtNavMeshParams params;
        file1.read( reinterpret_cast< char * >( &params ), sizeof( dtNavMeshParams ) );

        dtNavMesh navMesh;
        if ( navMesh.init( &params ) != DT_SUCCESS )
            return false;

        std::ifstream file2( Format( "F:\\Sunwell\\emu_data\\mmaps\\models\\%05u.mmtile", modelId ), std::ios::in | std::ios::binary );
        if ( !file2.is_open() )
            return false;

        std::vector< uint8_t > data;

        m_modelGeometry.emplace( modelId, DebugDetourDraw{} );
        LoadNavMeshTile( file2, navMesh, data, m_modelGeometry[ modelId ] );
        return true;
    }
}
