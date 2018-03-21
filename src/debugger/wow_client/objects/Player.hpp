#ifndef Player_hpp__
#define Player_hpp__

#include "memory/ProcessMemory.hpp"
#include "wow_client/Location.hpp"

namespace Wow
{
    class Player
    {
        enum Offsets
        {
            PTR_WorldMapId  = 0x00AB63BC,
            REL_Location    = 0x798
        };

    public:
        Player( Debugger::ProcessMemory & memory, uintptr_t offset );

        uint32_t GetMapId() const;
        Location GetLocation() const;

    private:
        Debugger::ProcessMemory &   m_memory;
        uintptr_t                   m_offset;
    };
}

#endif // Player_hpp__
