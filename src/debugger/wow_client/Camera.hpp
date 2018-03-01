#ifndef Camera_hpp__
#define Camera_hpp__

#include "memory/ProcessMemory.hpp"

#include "Location.hpp"

#include <cstdint>

namespace Wow
{
    class Camera
    {
    public:
        enum Offsets
        {
            PTR_ActiveCamera = 0xB7436C,
            REL_CameraOffset = 0x7E20
        };

        uint32_t    Unknown[ 2 ];

        Location    Position;
        float       ViewMatrix[ 3 ][ 3 ];
        float       NearPlane;
        float       FarPlane;
        float       FieldOfView;
        float       Aspect;
    };
}

#endif // Camera_hpp__
