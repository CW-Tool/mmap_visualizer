#ifndef ObjectManager_hpp__
#define ObjectManager_hpp__

#include "memory/ProcessMemory.hpp"

#include "Player.hpp"

#include "wow_client/Object.hpp"
#include "wow_client/Camera.hpp"

#include <cstdint>
#include <functional>
#include <optional>

namespace Wow
{
    class ObjectManager
    {
        enum Offsets
        {
            F_GetObjectByGuid       = 0x004D4DB0,
            F_GetLocalPlayerGuid    = 0x004D3790,

            F_EnumVisibleObjects    = 0x004D4B30,
        };

        using GetLocalPlayerGuid    = ObjectGuid( __cdecl * )();
        using GetObjectByGuid       = uintptr_t( __cdecl * )( ObjectGuid guid, int filter );
        using EnumVisibleObjects    = signed int( __cdecl * )( int( __cdecl *pCallback )( ObjectGuid, DWORD ), int filter );

    public:
        ObjectManager( Debugger::ProcessMemory & memory );

        bool                IsInWorld() const;

        Player              GetLocalPlayer() const;
        Camera              GetCamera() const;
        std::optional< Object > GetObject( ObjectGuid guid ) const;

        void                UpdateVisibleObjects();

        void                VisitVisibleObjects( std::function< void( ObjectGuid ) > && func );

    protected:
        std::vector< ObjectGuid > m_visibleObjects;

        GetLocalPlayerGuid  m_getLocalPlayerGuid;
        GetObjectByGuid     m_getObjectByGuid;
        EnumVisibleObjects  m_enumVisibleObjects;

        Debugger::ProcessMemory & m_memory;
    };
}

#endif // ObjectManager_hpp__
