#include "Debugger.hpp"

#include "renderer/Renderer.hpp"
#include "mmaps/NavMesh.hpp"

#include <iostream>
#include <fstream>
#include <vector>
#include "d3dx9math.h"

namespace Debugger
{
    template< typename ...Args >
    std::string Format( const char * format, Args... args )
    {
        size_t bufferSize = std::snprintf( nullptr, 0, format, std::forward< Args >( args )... ) + 1;

        std::string result;
        result.resize( bufferSize );

        bufferSize = std::snprintf( const_cast< char * >( result.c_str() ), result.size(), format, std::forward< Args >( args )... );
        result.resize( bufferSize );

        return result;
    }

    Debugger * g_debugger = nullptr;

    Debugger::Debugger( VTABLE_TYPE* vtable )
        : m_renderer( vtable )
        , m_memory( GetCurrentProcess() )
        , m_lua( m_memory )
        , m_objectMgr( m_memory )
    {
        g_debugger = this;

        m_lua.Execute( "AccountLoginAccountEdit:SetText('Root')" );
        m_lua.Execute( "AccountLoginPasswordEdit:SetText('Root')" );
        m_lua.Execute( "AccountLogin_Login()" );
    }

    inline int dtAlign4( int x )
    {
        return ( x + 3 ) & ~3;
    }

    std::optional< NavMesh > Debugger::LoadNavMesh( uint32_t mapId, const Vector2i & coord )
    {
        if ( m_lastLoadedNavMesh && m_lastLoadedNavMesh->mapId == mapId && m_lastLoadedNavMesh->coord.x == coord.x && m_lastLoadedNavMesh->coord.y == coord.y )
            return m_lastLoadedNavMesh;

        std::ifstream file( Format( "F:\\emu_data\\mmaps\\%03u%02u%02u.mmtile", mapId, coord.x, coord.y ), std::ios::in | std::ios::binary );
        if ( !file.is_open() )
            return std::nullopt;

        NavTileHeader tileHeader;
        file.read( reinterpret_cast< char * >( &tileHeader ), sizeof( NavTileHeader ) );

        m_lastLoadedNavMesh.emplace();

        NavMeshTile & tile = m_lastLoadedNavMesh->tile;
        std::vector< uint8_t > & data = m_lastLoadedNavMesh->data;

        data.resize( tileHeader.size );

        file.read( reinterpret_cast< char * >( &data[ 0 ] ), tileHeader.size );

        auto header = (NavMeshHeader*)&data[0];
        const int headerSize = dtAlign4( sizeof( NavMeshHeader ) );
        const int vertsSize = dtAlign4( sizeof( NavMeshVert ) * header->vertCount );
        const int polysSize = dtAlign4( sizeof( NavMeshPoly )*header->polyCount );
        const int linksSize = dtAlign4( sizeof( NavMeshLink )*( header->maxLinkCount ) );
        const int detailMeshesSize = dtAlign4( sizeof( NavMeshPolyDetail )*header->detailMeshCount );
        const int detailVertsSize = dtAlign4( sizeof( NavMeshVert ) * header->detailVertCount );
        const int detailTrisSize = dtAlign4( sizeof( unsigned char ) * 4 * header->detailTriCount );
        const int bvtreeSize = dtAlign4( sizeof( NavMeshBVNode )*header->bvNodeCount );
        const int offMeshLinksSize = dtAlign4( sizeof( NavMeshOffMeshConnection )*header->offMeshConCount );

        unsigned char* d = &data[0] + headerSize;
        tile.verts = ( float* )d; d += vertsSize;
        tile.polys = ( NavMeshPoly* )d; d += polysSize;
        tile.links = ( NavMeshLink* )d; d += linksSize;
        tile.detailMeshes = ( NavMeshPolyDetail* )d; d += detailMeshesSize;
        tile.detailVerts = ( float* )d; d += detailVertsSize;
        tile.detailTris = ( unsigned char* )d; d += detailTrisSize;
        tile.bvTree = ( NavMeshBVNode* )d; d += bvtreeSize;
        tile.offMeshCons = ( NavMeshOffMeshConnection* )d; d += offMeshLinksSize;

        // Build links freelist
        tile.linksFreeList = 0;
        tile.links[ header->maxLinkCount - 1 ].next = 0;
        for ( int i = 0; i < header->maxLinkCount - 1; ++i )
            tile.links[ i ].next = i + 1;

        // Init tile.
        tile.header = header;
        tile.data = &data[0];
        tile.dataSize = data.size();
        tile.flags = 0;

        return m_lastLoadedNavMesh;
    }

    Vector2i GetTileCoord( Wow::Location & loc )
    {
        const float     SIZE_OF_GRIDS       = 533.0f;
        const float     CENTER_GRID_OFFSET  = SIZE_OF_GRIDS / 2.0f;
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

        auto player = m_objectMgr.GetLocalPlayer();
        auto camera = m_objectMgr.GetCamera();

        uint32_t mapId = player.GetMapId();

        Wow::Location loc = player.GetLocation();
        Vector2i tile = GetTileCoord( loc );

        LoadNavMesh( mapId, tile );
    }

    void Debugger::RenderNavMesh()
    {
        struct NavDetailTriangle
        {
            uint8_t idx0;
            uint8_t idx1;
            uint8_t idx2;
            uint8_t unk;
        };

        if ( m_lastLoadedNavMesh )
        {
            auto camera = m_objectMgr.GetCamera();
            auto navMesh = m_lastLoadedNavMesh;

            NavMeshTile & tile = navMesh->tile;

            Vector3f * verts = reinterpret_cast< Vector3f * >( navMesh->tile.verts );
            Vector3f * detailVerts = reinterpret_cast< Vector3f * >( navMesh->tile.detailVerts );

            TriangleGeometry triangles( Colors::GreenAlpha );
            LineGeometry lines( Colors::WhiteAlpha );

            for ( auto polyIdx = 0u; polyIdx < navMesh->tile.header->polyCount; ++polyIdx )
            {
                NavMeshPoly & poly = reinterpret_cast< NavMeshPoly * >( tile.polys )[ polyIdx ];

                auto polyType = poly.areaAndtype >> 6;

                NavMeshPolyDetail & detail = reinterpret_cast< NavMeshPolyDetail * >( tile.detailMeshes )[ polyIdx ];
                for ( auto triIdx = 0u; triIdx < detail.triCount; ++triIdx )
                {
                    NavDetailTriangle & triangle = reinterpret_cast< NavDetailTriangle * >( tile.detailTris )[ detail.triBase + triIdx ];

                    uint8_t idx0 = triangle.idx0;
                    uint8_t idx1 = triangle.idx1;
                    uint8_t idx2 = triangle.idx2;

                    Vector3f & v0 = idx0 < poly.vertCount ? verts[ poly.verts[ idx0 ] ] : detailVerts[ ( detail.vertBase + ( idx0 - poly.vertCount ) ) ];
                    Vector3f & v1 = idx1 < poly.vertCount ? verts[ poly.verts[ idx1 ] ] : detailVerts[ ( detail.vertBase + ( idx1 - poly.vertCount ) ) ];
                    Vector3f & v2 = idx2 < poly.vertCount ? verts[ poly.verts[ idx2 ] ] : detailVerts[ ( detail.vertBase + ( idx2 - poly.vertCount ) ) ];

                    Vector3f vv0{ v0.z, v0.x, v0.y + 0.6f };
                    Vector3f vv1{ v1.z, v1.x, v1.y + 0.6f };
                    Vector3f vv2{ v2.z, v2.x, v2.y + 0.6f };

                    triangles.AddTriangle( vv0, vv1, vv2 );

                    lines.AddLine( vv0, vv1 );
                    lines.AddLine( vv0, vv2 );
                    lines.AddLine( vv1, vv2 );
                }
            }

            m_renderer.RenderGeometry( triangles, camera );
            m_renderer.RenderGeometry( lines, camera );
        }
    }

    extern Debugger * GetDebugger()
    {
        return g_debugger;
    }

}
