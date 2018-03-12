#ifndef Debugger_hpp__
#define Debugger_hpp__

#include "memory/ProcessMemory.hpp"
#include "renderer/Renderer.hpp"
#include "mmaps/NavMeshDebugger.hpp"

#include "wow_client/lua/LuaState.hpp"
#include "wow_client/objects/ObjectManager.hpp"

#include <vector>
#include <cstdint>

namespace Debugger
{
    class LuaFrame
    {
    public:
        LuaFrame( ProcessMemory & memory );

        bool           IsOpen() const { return m_isOpen; }
        void           Open();
        void           Close();

    protected:
        bool           m_isOpen;
        Wow::LuaState  m_lua;
    };

    Vector2i GetTileCoord( Wow::Location & loc );

    class Debugger
    {
    public:
        Debugger( VTABLE_TYPE* vtable );

        void                        Update();
        void                        Render();

        Wow::ObjectManager &        GetObjectMgr();

    private:

        Renderer            m_renderer;
        ProcessMemory       m_memory;

        NavMeshDebugger     m_navDebugger;
        Wow::ObjectManager  m_objectMgr;
    };

    extern Debugger * GetDebugger();
}

#endif // Debugger_hpp__
