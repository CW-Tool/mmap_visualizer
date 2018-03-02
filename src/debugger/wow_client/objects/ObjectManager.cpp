#include "ObjectManager.hpp"
#include <iostream>

namespace Wow
{
    ObjectManager::ObjectManager( Debugger::ProcessMemory & memory )
        : m_memory( memory )
    {
        m_getLocalPlayerGuid = reinterpret_cast< GetLocalPlayerGuid >( Offsets::F_GetLocalPlayerGuid );
        m_getObjectByGuid = reinterpret_cast< GetObjectByGuid >( Offsets::F_GetObjectByGuid );

        auto guid = m_getLocalPlayerGuid();
    }

    bool ObjectManager::IsInWorld() const
    {
        return m_getLocalPlayerGuid() != 0u;
    }

    Player ObjectManager::GetLocalPlayer() const
    {
        ObjectGuid playerGUID = m_getLocalPlayerGuid();

        uintptr_t offset = m_getObjectByGuid( playerGUID, -1 );
        return Player( m_memory, offset );
    }

    Camera ObjectManager::GetCamera() const
    {
        uintptr_t cameraOffset = m_memory.Read< uintptr_t >( m_memory.Read< uintptr_t >( Camera::PTR_ActiveCamera ) + Camera::REL_CameraOffset );
        return Camera( m_memory, cameraOffset );
    }
}
