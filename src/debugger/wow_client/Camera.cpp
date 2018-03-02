#include "Camera.hpp"

namespace Wow
{
    Camera::Camera( Debugger::ProcessMemory & memory, uintptr_t offset )
        : m_memory( memory )
        , m_offset( offset )
    {
        m_memory.ReadInto< CameraInfo >( *reinterpret_cast< CameraInfo* >( &Unknown[ 0 ] ), offset );

        m_getForwardVec = reinterpret_cast< GetVector3 >( m_memory.GetFunctionFromVtable( m_offset, VTable::IDX_GetForwardVector ) );
        m_getRightVec = reinterpret_cast< GetVector3 >( m_memory.GetFunctionFromVtable( m_offset, VTable::IDX_GetRightVector ) );
        m_getUpVec = reinterpret_cast< GetVector3 >( m_memory.GetFunctionFromVtable( m_offset, VTable::IDX_GetUpVector ) );
    }

    Vector3f Camera::GetForwardDir() const
    {
        Vector3f result;
        m_getForwardVec( m_offset, &result );

        return result;
    }

    Vector3f Camera::GetRightDir() const
    {
        Vector3f result;
        m_getRightVec( m_offset, &result );

        return result;
    }

    Vector3f Camera::GetUpDir() const
    {
        Vector3f result;
        m_getUpVec( m_offset, &result );

        return result;
    }
}
