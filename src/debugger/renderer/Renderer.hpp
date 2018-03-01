#ifndef Renderer_hpp__
#define Renderer_hpp__

#include <windows.h>
#include <d3d9.h>

#include <optional>

namespace Wow
{
    class Camera;
    class Location;
}

namespace Debugger
{
    class Debugger;

#if defined _M_X64
    using VTABLE_TYPE = DWORD64;
#elif defined _M_IX86
    using VTABLE_TYPE = DWORD;
#endif

    using EndSceneFunc = HRESULT( APIENTRY* ) ( IDirect3DDevice9* );

    enum VTable
    {
        Reset                       = 16,
        EndScene                    = 42,
        SetViewport                 = 47,
        SetTexture                  = 65,
        DrawPrimitive               = 81,
        DrawIndexedPrimitive        = 82,
        SetVertexDeclaration        = 87,
        SetVertexShader             = 92,
        SetVertexShaderConstantF    = 94,
        SetStreamSource             = 100,
        SetIndices                  = 104,
        SetPixelShader              = 107,
    };

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

    class Renderer
    {
    public:
        Renderer( VTABLE_TYPE* vtable, Debugger * debugger );
        ~Renderer();

        void SetupDirectXHooks( VTABLE_TYPE* vtable );

        void EndScene();

        std::optional< Vector2f > GetScreenCoord( Wow::Camera & camera, Wow::Location & location );

        void                      RenderQuad( float x, float y, float width, float height );
        void                      RenderLine( Vector2f start, Vector2f end );

    protected:
        IDirect3DDevice9 *  m_device;
        static Renderer *   s_renderer;

        Debugger * m_debugger;
    };
}

#endif // Renderer_hpp__
