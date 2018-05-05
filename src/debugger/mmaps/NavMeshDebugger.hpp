#ifndef NavMeshDebugger_hpp__
#define NavMeshDebugger_hpp__

#include "renderer/Renderer.hpp"

#include "math/Vector.hpp"
#include "DetourNavMesh.h"

#include "wow_client/objects/Player.hpp"
#include "wow_client/Camera.hpp"

#include <vector>
#include <optional>
#include <unordered_map>

namespace Debugger
{
    using NavMeshGeometryMap = std::unordered_map< uint32_t, DebugDetourDraw >;

    class NavMeshDebugger
    {
    public:
        NavMeshDebugger();

        void                    Render();
        void                    RenderModel( uint32_t modelId, const Wow::Location & pos, float rot );

        void                    Update( Wow::Player & player, Wow::Camera & camera );
        void                    Clear();

    protected:
        bool                    LoadMapNavMesh( uint32_t mapId, const Vector2i & coord );
        bool                    LoadModelNavMesh( uint32_t modelId );

        NavMeshGeometryMap      m_modelGeometry;
        NavMeshGeometryMap      m_mapGeometry;
    };
}

#endif // NavMeshDebugger_hpp__