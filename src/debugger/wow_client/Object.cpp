#include "Object.hpp"

namespace Wow
{
    Object::Object( Debugger::ProcessMemory & memory, uintptr_t offset )
        : m_offset( offset )
        , m_memory( memory )
    {
        m_getPosition = reinterpret_cast< _GetPosition >( m_memory.GetFunctionFromVtable( m_offset, VTable::IDX_GetPosition ) );
        m_getFacing = reinterpret_cast< _GetFacing >( m_memory.GetFunctionFromVtable( m_offset, VTable::IDX_GetFacing ) );
    }

    Location Object::GetPositon() const
    {
        Location position{};
        m_getPosition( m_offset, &position.x );

        return position;
    }

    float Object::GetFacing() const
    {
        return m_getFacing( m_offset );
    }
}
