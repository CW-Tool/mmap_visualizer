#ifndef Object_hpp__
#define Object_hpp__

#include "Location.hpp"

#include <cstdint>
#include "memory/ProcessMemory.hpp"

namespace Wow
{
    using ObjectGuid = uint64_t;

    enum ObjectFields
    {
        OBJECT_FIELD_GUID           = 0x0,
        OBJECT_FIELD_TYPE           = 0x2,
        OBJECT_FIELD_ENTRY          = 0x3,
        OBJECT_FIELD_SCALE_X        = 0x4,
        OBJECT_FIELD_PADDING        = 0x5,

        OBJECT_END                  = 0x6,

        GAMEOBJECT_DISPLAYID        = OBJECT_END + 0x0002, // Size: 1, Type: INT, Flags: PUBLIC
        GAMEOBJECT_FLAGS            = OBJECT_END + 0x0003, // Size: 1, Type: INT, Flags: PUBLIC
        GAMEOBJECT_PARENTROTATION   = OBJECT_END + 0x0004, // Size: 4, Type: FLOAT, Flags: PUBLIC
        GAMEOBJECT_DYNAMIC          = OBJECT_END + 0x0008, // Size: 1, Type: TWO_SHORT, Flags: DYNAMIC
        GAMEOBJECT_FACTION          = OBJECT_END + 0x0009, // Size: 1, Type: INT, Flags: PUBLIC
        GAMEOBJECT_LEVEL            = OBJECT_END + 0x000A, // Size: 1, Type: INT, Flags: PUBLIC
        GAMEOBJECT_BYTES_1          = OBJECT_END + 0x000B, // Size: 1, Type: BYTES, Flags: PUBLIC
        GAMEOBJECT_END              = OBJECT_END + 0x000C,
    };

    enum ObjectType
    {
        OBJECT_TYPE_OBJECT         = 0x0001,
        OBJECT_TYPE_ITEM           = 0x0002,
        OBJECT_TYPE_CONTAINER      = 0x0006,
        OBJECT_TYPE_UNIT           = 0x0008,
        OBJECT_TYPE_PLAYER         = 0x0010,
        OBJECT_TYPE_GAMEOBJECT     = 0x0020,
        OBJECT_TYPE_DYNAMICOBJECT  = 0x0040,
        OBJECT_TYPE_CORPSE         = 0x0080,
    };

    class Object
    {
    public:
        enum VTable
        {
            IDX_GetPosition = 12,
            IDX_GetFacing   = 14,
            IDX_GetModel    = 24
        };

        Object( Debugger::ProcessMemory & memory, uintptr_t offset );

        Location                    GetPositon() const;
        float                       GetFacing() const;

        template< class T >
        T GetField( ObjectFields field )
        {
             return *reinterpret_cast< T * >( &m_memory.Read< uint8_t* >( m_offset + 0x8 )[ field * 4 ] );
        }

    protected:
        using _GetPosition          = void( __thiscall* )( uintptr_t, float * );
        using _GetFacing            = float( __thiscall* )( uintptr_t );
        using _GetModel             = uint32_t( __thiscall* )( uintptr_t );

        _GetModel                   m_getModel;
        _GetPosition                m_getPosition;
        _GetFacing                  m_getFacing;

        uintptr_t                   m_offset;
        Debugger::ProcessMemory &   m_memory;
    };
}

#endif // Object_hpp__
