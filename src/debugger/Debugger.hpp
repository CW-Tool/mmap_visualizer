#ifndef Debugger_hpp__
#define Debugger_hpp__

#include "memory/ProcessMemory.hpp"
#include "renderer/Renderer.hpp"

#include "wow_client/LuaState.hpp"

namespace Debugger
{
    class Debugger
    {
    public:
        Debugger( VTABLE_TYPE* vtable );

    private:
        Renderer      m_renderer;
        ProcessMemory m_memory;

        Wow::LuaState m_lua;
    };
}

#endif // Debugger_hpp__
