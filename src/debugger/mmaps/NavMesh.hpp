#ifndef NavMesh_hpp__
#define NavMesh_hpp__

#include <cstdint>

namespace Debugger
{
    struct NavTileHeader
    {
        uint32_t    mmapMagic;
        uint32_t    dtVersion;
        uint32_t    mmapVersion;
        uint32_t    size;
        bool        usesLiquids : 1;
    };
}

#endif // NavMesh_hpp__
