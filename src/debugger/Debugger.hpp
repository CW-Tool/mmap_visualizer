#ifndef Debugger_hpp__
#define Debugger_hpp__

#include "memory/ProcessMemory.hpp"
#include "renderer/Renderer.hpp"
#include "mmaps/NavMeshDebugger.hpp"
#include "sniffer/PacketSniffer.hpp"

#include "wow_client/lua/LuaState.hpp"
#include "wow_client/objects/ObjectManager.hpp"

#include "MinHook.h"

#include <vector>
#include <cstdint>
#include <fstream>

namespace Debugger
{
    template< class F >
    void RegisterHook( uintptr_t offset, F && hook )
    {
        using FunctionType = F;

        LPVOID address = reinterpret_cast< LPVOID >( offset );
        static FunctionType s_OrigFunc = reinterpret_cast< FunctionType >( address );
        static FunctionType s_HookFunc = std::forward< FunctionType >( hook );

        static FunctionType s_RealHook = []( auto ... args )
        {
            s_HookFunc( args... );

            return s_OrigFunc( args... );
        };

        MH_CreateHook( address, s_RealHook, reinterpret_cast< void** >( &s_OrigFunc ) );
        MH_EnableHook( address );
    }

    class DebuggerLuaFrame
    {
    public:
        DebuggerLuaFrame( ProcessMemory & memory );

        bool           IsOpen() const { return m_isOpen; }
        bool           IsNavMeshVisible();
        bool           IsPathRenderingEnabled();

        void           Open();
        void           Close();

    protected:
        bool                m_isOpen;

        Wow::LuaState       m_lua;
        Wow::LuaCheckBox *  m_navMeshBox;
        Wow::LuaCheckBox *  m_pathRenderingBox;
    };

    Vector2i GetTileCoord( Wow::Location & loc );

    class Debugger
    {
    public:
        Debugger( VTABLE_TYPE* vtable );

        void                        Update();
        void                        Render();

        void                        Log( const std::string& msg );

        Wow::ObjectManager &        GetObjectMgr();

    private:
        std::ofstream       m_logFile;

        Renderer            m_renderer;
        ProcessMemory       m_memory;
        PacketSniffer       m_sniffer;

        struct PathGeometry
        {
            Wow::ObjectGuid                 m_transportGuid;
            std::unique_ptr< LineGeometry > m_geometry;
        };

        std::unordered_map< Wow::ObjectGuid, std::vector< PathGeometry > > m_paths;
        std::unique_ptr< DebuggerLuaFrame > m_frame;
        NavMeshDebugger     m_navDebugger;
        Wow::ObjectManager  m_objectMgr;
    };

    extern Debugger *       GetDebugger();
}

#endif // Debugger_hpp__
