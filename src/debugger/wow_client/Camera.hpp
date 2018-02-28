#ifndef Camera_hpp__
#define Camera_hpp__

#include "Location.hpp"

namespace Wow
{
    struct Camera
    {
        uint32_t    Unknown[ 2 ];

        Location    Position;
        float       ViewMatrix[ 9 ];
        float       NearPlane;
        float       FarPlane;
        float       FieldOfView;
        float       Aspect;
    };
}

#endif // Camera_hpp__
