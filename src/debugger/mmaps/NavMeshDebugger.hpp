#ifndef NavMeshDebugger_hpp__
#define NavMeshDebugger_hpp__

#include "renderer/Renderer.hpp"

#include "math/Vector.hpp"
#include "NavMesh.hpp"

#include "wow_client/objects/Player.hpp"
#include "wow_client/Camera.hpp"

#include <vector>
#include <optional>
#include <unordered_map>

namespace Debugger
{
    struct MapNavMesh
    {
        uint32_t mapId;
        Vector2i coord;

        std::vector< uint8_t > data;
        NavMeshTile tile;
    };

    struct NavMeshGeometry
    {
        NavMeshGeometry( Colors color = Colors::GreenAlpha )
            : triangles( Colors::GreenAlpha )
            , lines( Colors::WhiteAlpha )
        {
        }

        TriangleGeometry    triangles;
        LineGeometry        lines;
    };

    using NavMeshGeometryMap = std::unordered_map< uint32_t, NavMeshGeometry >;

    class NavMeshDebugger
    {
    public:
        NavMeshDebugger();

        bool                     IsEnabled() const;
        void                     SetEnabled( bool isEnabled );

        void                     Render();
        void                     RenderModel( uint32_t modelId, const Wow::Location & pos, float rot );

        void                     Update( Wow::Player & player, Wow::Camera & camera );

    protected:
        std::optional< MapNavMesh > LoadMapNavMesh( uint32_t mapId, const Vector2i & coord );
        bool                        LoadModelNavMesh( uint32_t modelId );

        std::optional< MapNavMesh > m_lastLoadedNavMesh;
        NavMeshGeometryMap          m_modelGeometry;

        bool                        m_isEnabled;
        NavMeshGeometry             m_geometry;
    };
}

#endif // NavMeshDebugger_hpp__