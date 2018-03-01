#include "Debugger.hpp"

#include "renderer/Renderer.hpp"
#include "mmaps/NavMesh.hpp"

#include <iostream>
#include <fstream>
#include <vector>

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

    Debugger::Debugger( VTABLE_TYPE* vtable )
        : m_renderer( vtable, this )
        , m_memory( GetCurrentProcess() )
        , m_lua( m_memory )
        , m_objectMgr( m_memory )
    {
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

        Wow::Location loc = player.GetLocation();

        auto coord = m_renderer.GetScreenCoord( camera, loc );
        if ( coord )
        {
            m_renderer.RenderQuad( coord->x, coord->y, 10.0f, 10.0f );
        }

        uint32_t mapId = player.GetMapId();
        Vector2i tile = GetTileCoord( loc );

        auto navMesh = LoadNavMesh( mapId, tile );
        if ( navMesh )
        {
            Vector3f * verts = reinterpret_cast< Vector3f * >( navMesh->tile.verts );

            //for ( auto idx = 0u; idx < navMesh->tile.header->vertCount; ++idx )
            //{
            //    Wow::Location loc{ pos->z, pos->x, pos->y };

            //    auto coord = m_renderer.GetScreenCoord( camera, loc );
            //    if ( coord )
            //    {
            //        m_renderer.RenderQuad( coord->x, coord->y, 3.0f, 3.0f );
            //    }

            //    ++pos;
            //}

            NavMeshPoly * poly = reinterpret_cast< NavMeshPoly * >( navMesh->tile.polys );
            for ( auto idx = 0u; idx < navMesh->tile.header->polyCount; ++idx )
            {
                for ( auto vertIdx = 0u; vertIdx < poly->vertCount - 1; ++vertIdx )
                {
                    auto vertIdx0 = poly->verts[ vertIdx ];
                    auto vertIdx1 = poly->verts[ vertIdx + 1 ];

                    Wow::Location loc0{ verts[ vertIdx0 ].z, verts[ vertIdx0 ].x, verts[ vertIdx0 ].y };
                    Wow::Location loc1{ verts[ vertIdx1 ].z, verts[ vertIdx1 ].x, verts[ vertIdx1 ].y };

                    auto coord0 = m_renderer.GetScreenCoord( camera, loc0 );
                    auto coord1 = m_renderer.GetScreenCoord( camera, loc1 );
                    if ( coord0 && coord1 )
                    {
                        m_renderer.RenderLine( *coord0, *coord1 );
                    }
                }

                ++poly;
            }
        }
    }
}
