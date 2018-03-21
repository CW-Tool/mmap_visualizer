#ifndef Debugger_hpp__
#define Debugger_hpp__

#include "memory/ProcessMemory.hpp"
#include "renderer/Renderer.hpp"
#include "mmaps/NavMeshDebugger.hpp"

#include "wow_client/lua/LuaState.hpp"
#include "wow_client/objects/ObjectManager.hpp"

#include <vector>
#include <cstdint>
#include <fstream>

namespace Debugger
{
    class DebuggerLuaFrame
    {
    public:
        DebuggerLuaFrame( ProcessMemory & memory );

        bool           IsOpen() const { return m_isOpen; }
        bool           IsNavMeshVisible();

        void           Open();
        void           Close();

    protected:
        bool                m_isOpen;

        Wow::LuaState       m_lua;
        Wow::LuaCheckBox *  m_navMeshBox;
    };

    Vector2i GetTileCoord( Wow::Location & loc );

    class Debugger
    {
    public:
        Debugger( VTABLE_TYPE* vtable );

        void                        Update();
        void                        Render();

        void                        Log( const std::string& msg );

        static void                 RegisterLua( Wow::LuaState & state );

        Wow::ObjectManager &        GetObjectMgr();

    private:
        std::ofstream       m_logFile;

        Renderer            m_renderer;
        ProcessMemory       m_memory;

        NavMeshDebugger     m_navDebugger;
        Wow::ObjectManager  m_objectMgr;
    };

    extern Debugger *       GetDebugger();
}

#endif // Debugger_hpp__
