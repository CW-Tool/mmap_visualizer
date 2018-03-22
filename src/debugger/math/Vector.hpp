#ifndef Vector_hpp__
#define Vector_hpp__

#include <cstdint>

struct Vector2f
{
    float x;
    float y;
};

struct Vector3f
{
    float x;
    float y;
    float z;

    inline Vector3f operator+( const Vector3f & rhs )
    {
        return Vector3f{ x + rhs.x, y + rhs.y, z + rhs.z };
    }

    inline Vector3f operator-( const Vector3f & rhs )
    {
        return Vector3f{ x - rhs.x, y - rhs.y, z - rhs.z };
    }
};

struct Vector2u
{
    uint32_t x;
    uint32_t y;
};

struct Vector2i
{
    int32_t x;
    int32_t y;
};

#endif // Vector_hpp__
