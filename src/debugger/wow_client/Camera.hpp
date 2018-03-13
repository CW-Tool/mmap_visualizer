#ifndef Camera_hpp__
#define Camera_hpp__

#include "memory/ProcessMemory.hpp"
#include "math/Vector.hpp"

#include "Location.hpp"

#include <cstdint>

namespace Wow
{
    class CameraInfo
    {
    public:
        enum Offsets
        {
            PTR_ActiveCamera = 0xB7436C,
            REL_CameraOffset = 0x7E20
        };

        enum VTable
        {
            IDX_GetForwardVector    = 1,
            IDX_GetRightVector      = 2,
            IDX_GetUpVector         = 3,
        };

        uint32_t    Unknown[ 2 ];

        Location    Position;
        float       Facing[ 9 ];
        float       NearPlane;
        float       FarPlane;
        float       FieldOfView;
        float       Aspect;
    };

    using GetVector3 = Vector3f * ( __thiscall  * )( uintptr_t, Vector3f* );

    class Camera : public CameraInfo
    {
    public:
        Camera( Debugger::ProcessMemory & memory, uintptr_t offset );

        Vector3f                  GetForwardDir() const;
        Vector3f                  GetRightDir() const;
        Vector3f                  GetUpDir() const;

    private:
        uintptr_t                 m_offset;
        Debugger::ProcessMemory & m_memory;

        GetVector3                m_getForwardVec;
        GetVector3                m_getRightVec;
        GetVector3                m_getUpVec;
    };
}

#endif // Camera_hpp__
