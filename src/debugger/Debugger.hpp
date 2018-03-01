#ifndef Debugger_hpp__
#define Debugger_hpp__

#include "memory/ProcessMemory.hpp"
#include "renderer/Renderer.hpp"
#include "mmaps/NavMesh.hpp"

#include "wow_client/lua/LuaState.hpp"
#include "wow_client/objects/ObjectManager.hpp"

#include <vector>
#include <cstdint>

namespace Debugger
{
    struct NavMesh
    {
        uint32_t mapId;
        Vector2i coord;

        std::vector< uint8_t > data;
        NavMeshTile tile;
    };

    class Debugger
    {
    public:
        Debugger( VTABLE_TYPE* vtable );

        std::optional< NavMesh >    LoadNavMesh( uint32_t mapId, const Vector2i & coord );

        void                        Update();

    private:
        std::optional< NavMesh > m_lastLoadedNavMesh;

        Renderer            m_renderer;
        ProcessMemory       m_memory;

        Wow::ObjectManager  m_objectMgr;
        Wow::LuaState       m_lua;
    };
}

#endif // Debugger_hpp__
