#include "NavMeshDebugger.hpp"

#include "Debugger.hpp"
#include "renderer/Renderer.hpp"

#include "math/Vector.hpp"
#include "utils/String.hpp"

#include <fstream>

namespace Debugger
{
    inline int dtAlign4( int x )
    {
        return ( x + 3 ) & ~3;
    }

    NavMeshDebugger::NavMeshDebugger()
        : m_isEnabled( true )
    {
    }

    bool NavMeshDebugger::IsEnabled() const
    {
        return m_isEnabled;
    }

    void NavMeshDebugger::SetEnabled( bool isEnabled )
    {
        m_isEnabled = isEnabled;
    }

    void NavMeshDebugger::Render()
    {
        if ( m_lastLoadedNavMesh )
        {
            Wow::Camera camera = GetDebugger()->GetObjectMgr().GetCamera();
            Transform transform = Transform::FromCamera( camera );

            auto renderer = GetRenderer();
            renderer->RenderGeometry( m_geometry.triangles, &transform );
            renderer->RenderGeometry( m_geometry.lines, &transform );
        }
    }

    void NavMeshDebugger::Update( Wow::Player & player, Wow::Camera & camera )
    {
        uint32_t mapId = player.GetMapId();

        Wow::Location loc = player.GetLocation();
        Vector2i tile = GetTileCoord( loc );

        m_lastLoadedNavMesh = LoadNavMesh( mapId, tile );
    }

    std::optional< NavMesh > NavMeshDebugger::LoadNavMesh( uint32_t mapId, const Vector2i & coord )
    {
        if ( m_lastLoadedNavMesh && m_lastLoadedNavMesh->mapId == mapId && m_lastLoadedNavMesh->coord.x == coord.x && m_lastLoadedNavMesh->coord.y == coord.y )
            return m_lastLoadedNavMesh;

        std::ifstream file( Format( "F:\\Sunwell\\emu_data\\mmaps\\%03u%02u%02u.mmtile", mapId, coord.x, coord.y ), std::ios::in | std::ios::binary );
        if ( !file.is_open() )
            return std::nullopt;

        NavTileHeader tileHeader;
        file.read( reinterpret_cast< char * >( &tileHeader ), sizeof( NavTileHeader ) );

        m_lastLoadedNavMesh.emplace();

        NavMeshTile & tile = m_lastLoadedNavMesh->tile;
        std::vector< uint8_t > & data = m_lastLoadedNavMesh->data;

        data.resize( tileHeader.size );

        file.read( reinterpret_cast< char * >( &data[ 0 ] ), tileHeader.size );

        auto header = ( NavMeshHeader* )&data[ 0 ];
        const int headerSize = dtAlign4( sizeof( NavMeshHeader ) );
        const int vertsSize = dtAlign4( sizeof( NavMeshVert ) * header->vertCount );
        const int polysSize = dtAlign4( sizeof( NavMeshPoly )*header->polyCount );
        const int linksSize = dtAlign4( sizeof( NavMeshLink )*( header->maxLinkCount ) );
        const int detailMeshesSize = dtAlign4( sizeof( NavMeshPolyDetail )*header->detailMeshCount );
        const int detailVertsSize = dtAlign4( sizeof( NavMeshVert ) * header->detailVertCount );
        const int detailTrisSize = dtAlign4( sizeof( unsigned char ) * 4 * header->detailTriCount );
        const int bvtreeSize = dtAlign4( sizeof( NavMeshBVNode )*header->bvNodeCount );
        const int offMeshLinksSize = dtAlign4( sizeof( NavMeshOffMeshConnection )*header->offMeshConCount );

        unsigned char* d = &data[ 0 ] + headerSize;
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
        tile.data = &data[ 0 ];
        tile.dataSize = data.size();
        tile.flags = 0;

        //! build renderable
        {
            struct NavDetailTriangle
            {
                uint8_t idx0;
                uint8_t idx1;
                uint8_t idx2;
                uint8_t unk;
            };

            Vector3f * verts = reinterpret_cast< Vector3f * >( m_lastLoadedNavMesh->tile.verts );
            Vector3f * detailVerts = reinterpret_cast< Vector3f * >( m_lastLoadedNavMesh->tile.detailVerts );

            TriangleGeometry & triangles = m_geometry.triangles;
            triangles.Clear();

            LineGeometry & lines = m_geometry.lines;
            lines.Clear();

            for ( int polyIdx = 0; polyIdx < m_lastLoadedNavMesh->tile.header->detailMeshCount; ++polyIdx )
            {
                NavMeshPoly & poly = reinterpret_cast< NavMeshPoly * >( tile.polys )[ polyIdx ];

                auto polyType = poly.areaAndtype >> 6;

                NavMeshPolyDetail & detail = reinterpret_cast< NavMeshPolyDetail * >( tile.detailMeshes )[ polyIdx ];
                for ( int triIdx = 0; triIdx < detail.triCount; ++triIdx )
                {
                    NavDetailTriangle & triangle = reinterpret_cast< NavDetailTriangle * >( tile.detailTris )[ detail.triBase + triIdx ];

                    uint8_t idx0 = triangle.idx0;
                    uint8_t idx1 = triangle.idx1;
                    uint8_t idx2 = triangle.idx2;

                    Vector3f & v0 = idx0 < poly.vertCount ? verts[ poly.verts[ idx0 ] ] : detailVerts[ ( detail.vertBase + ( idx0 - poly.vertCount ) ) ];
                    Vector3f & v1 = idx1 < poly.vertCount ? verts[ poly.verts[ idx1 ] ] : detailVerts[ ( detail.vertBase + ( idx1 - poly.vertCount ) ) ];
                    Vector3f & v2 = idx2 < poly.vertCount ? verts[ poly.verts[ idx2 ] ] : detailVerts[ ( detail.vertBase + ( idx2 - poly.vertCount ) ) ];

                    Vector3f vv0{ v0.z, v0.x, v0.y };
                    Vector3f vv1{ v1.z, v1.x, v1.y };
                    Vector3f vv2{ v2.z, v2.x, v2.y };

                    triangles.AddTriangle( vv0, vv1, vv2 );

                    lines.AddLine( vv0, vv1 );
                    lines.AddLine( vv0, vv2 );
                    lines.AddLine( vv1, vv2 );
                }
            }
        }

        return m_lastLoadedNavMesh;
    }
}
