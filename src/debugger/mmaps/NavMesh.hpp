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

    struct NavMeshHeader
    {
        int magic;								// Magic number, used to identify the data.
        int version;							// Data version number.
        int x, y;								// Location of the time on the grid.
        unsigned int userId;					// User ID of the tile.
        int polyCount;							// Number of polygons in the tile.
        int vertCount;							// Number of vertices in the tile.
        int maxLinkCount;						// Number of allocated links.
        int detailMeshCount;					// Number of detail meshes.
        int detailVertCount;					// Number of detail vertices.
        int detailTriCount;						// Number of detail triangles.
        int bvNodeCount;						// Number of BVtree nodes.
        int offMeshConCount;					// Number of Off-Mesh links.
        int offMeshBase;						// Index to first polygon which is Off-Mesh link.
        float walkableHeight;					// Height of the agent.
        float walkableRadius;					// Radius of the agent
        float walkableClimb;					// Max climb height of the agent.
        float bmin[ 3 ], bmax[ 3 ];				// Bounding box of the tile.
        float bvQuantFactor;					// BVtree quantization factor (world to bvnode coords)
    };

    struct NavMeshVert
    {
        float x;
        float y;
        float z;
    };

    static const int DT_VERTS_PER_POLYGON = 6;

    struct NavMeshPoly
    {
        unsigned int firstLink;						    // Index to first link in linked list. 
        unsigned short verts[ DT_VERTS_PER_POLYGON ];	// Indices to vertices of the poly.
        unsigned short neis[ DT_VERTS_PER_POLYGON ];	// Refs to neighbours of the poly.
        unsigned short flags;						    // Flags (see dtPolyFlags).
        unsigned char vertCount;					    // Number of vertices.
        unsigned char areaAndtype;					    // Bit packed: Area ID of the polygon, and Polygon type, see dtPolyTypes..
    };

    struct NavMeshPolyDetail
    {
        unsigned int vertBase;						// Offset to detail vertex array.
        unsigned int triBase;						// Offset to detail triangle array.
        unsigned char vertCount;					// Number of vertices in the detail mesh.
        unsigned char triCount;						// Number of triangles.
    };

    struct NavMeshLink
    {
        uint64_t ref;							// Neighbour reference.
        unsigned int next;						// Index to next link.
        unsigned char edge;						// Index to polygon edge which owns this link. 
        unsigned char side;						// If boundary link, defines on which side the link is.
        unsigned char bmin, bmax;				// If boundary link, defines the sub edge area.
    };

    struct NavMeshBVNode
    {
        unsigned short bmin[ 3 ], bmax[ 3 ];		// BVnode bounds
        int i;									// Index to item or if negative, escape index.
    };

    struct NavMeshOffMeshConnection
    {
        float pos[ 6 ];							// Both end point locations.
        float rad;								// Link connection radius.
        unsigned short poly;					// Poly Id
        unsigned char flags;					// Link flags
        unsigned char side;						// End point side.
        unsigned int userId;					// User ID to identify this connection.
    };

    struct NavMeshTile
    {
        unsigned int salt;						// Counter describing modifications to the tile.

        unsigned int linksFreeList;				// Index to next free link.
        NavMeshHeader* header;					// Pointer to tile header.
        NavMeshPoly* polys;							// Pointer to the polygons (will be updated when tile is added).
        float* verts;							// Pointer to the vertices (will be updated when tile added).
        NavMeshLink* links;							// Pointer to the links (will be updated when tile added).
        NavMeshPolyDetail* detailMeshes;				// Pointer to detail meshes (will be updated when tile added).
        float* detailVerts;						// Pointer to detail vertices (will be updated when tile added).
        unsigned char* detailTris;				// Pointer to detail triangles (will be updated when tile added).
        NavMeshBVNode* bvTree;						// Pointer to BVtree nodes (will be updated when tile added).
        NavMeshOffMeshConnection* offMeshCons;		// Pointer to Off-Mesh links. (will be updated when tile added).

        unsigned char* data;					// Pointer to tile data.
        int dataSize;							// Size of the tile data.
        int flags;								// Tile flags, see dtTileFlags.
        NavMeshTile* next;						// Next free tile or, next tile in spatial grid.
    };
}

#endif // NavMesh_hpp__
