#include "Player.hpp"

namespace Wow
{
    Player::Player( Debugger::ProcessMemory & memory, uintptr_t offset )
        : m_memory( memory )
        , m_offset( offset )
    {

    }

    uint32_t Player::GetMapId() const
    {
        return m_memory.Read< uint32_t >( Offsets::PTR_WorldMapId );
    }

    Location Player::GetLocation() const
    {
        return m_memory.Read< Location >( m_offset + Offsets::REL_Location );
    }
}
