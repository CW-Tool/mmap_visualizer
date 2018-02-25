#ifndef Renderer_hpp__
#define Renderer_hpp__

#include <windows.h>
#include <d3d9.h>

namespace Debugger
{
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

    class Renderer
    {
    public:
        Renderer( VTABLE_TYPE* vtable );
        ~Renderer();

        void SetupDirectXHooks( VTABLE_TYPE* vtable );

        void EndScene();

        void RenderQuad( float x, float y, float width, float height );

    protected:
        IDirect3DDevice9 *  m_device;
        static Renderer *   s_renderer;
    };
}

#endif // Renderer_hpp__
