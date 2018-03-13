#ifndef NavMeshDebugger_hpp__
#define NavMeshDebugger_hpp__

#include "renderer/Renderer.hpp"

#include "math/Vector.hpp"
#include "NavMesh.hpp"

#include "wow_client/objects/Player.hpp"
#include "wow_client/Camera.hpp"

#include <vector>
#include <optional>

namespace Debugger
{
    struct NavMesh
    {
        uint32_t mapId;
        Vector2i coord;

        std::vector< uint8_t > data;
        NavMeshTile tile;
    };

    struct NavMeshGeometry
    {
        NavMeshGeometry()
            : triangles( Colors::GreenAlpha )
            , lines( Colors::WhiteAlpha )
        {
        }

        TriangleGeometry    triangles;
        LineGeometry        lines;
    };

    class NavMeshDebugger
    {
    public:
        NavMeshDebugger();

        bool                     IsEnabled() const;
        void                     SetEnabled( bool isEnabled );

        void                     Render();
        void                     Update( Wow::Player & player, Wow::Camera & camera );

    protected:
        std::optional< NavMesh > LoadNavMesh( uint32_t mapId, const Vector2i & coord );

        std::optional< NavMesh > m_lastLoadedNavMesh;

        bool                     m_isEnabled;
        NavMeshGeometry          m_geometry;
    };
}

#endif // NavMeshDebugger_hpp__