#ifndef ObjectManager_hpp__
#define ObjectManager_hpp__

#include "memory/ProcessMemory.hpp"

#include "Player.hpp"
#include "wow_client/Camera.hpp"

#include <cstdint>

namespace Wow
{
    using ObjectGuid = uint64_t;

    class ObjectManager
    {
        enum Offsets
        {
            F_GetObjectByGuid       = 0x004D4DB0,
            F_GetLocalPlayerGuid    = 0x004D3790,
        };

        using GetLocalPlayerGuid    = ObjectGuid( __cdecl * )();
        using GetObjectByGuid       = uintptr_t( __cdecl * )( ObjectGuid guid, int filter );

    public:
        ObjectManager( Debugger::ProcessMemory & memory );

        bool                IsInWorld() const;

        Player              GetLocalPlayer() const;
        Camera              GetCamera() const;

    protected:
        GetLocalPlayerGuid  m_getLocalPlayerGuid;
        GetObjectByGuid     m_getObjectByGuid;

        Debugger::ProcessMemory & m_memory;
    };
}

#endif // ObjectManager_hpp__
