#ifndef Renderer_hpp__
#define Renderer_hpp__

#include "math/Vector.hpp"
#include "wow_client/Camera.hpp"

#include <windows.h>
#include <d3d9.h>
#include <d3dx9math.h>

#include <optional>
#include <vector>
#include <stack>
#include <array>

namespace Debugger
{
#if defined _M_X64
    using VTABLE_TYPE = DWORD64;
#elif defined _M_IX86
    using VTABLE_TYPE = DWORD;
#endif

    namespace D3D9
    {
        using BeginScene = HRESULT( APIENTRY* )( IDirect3DDevice9* );
        using EndScene = HRESULT( APIENTRY* )( IDirect3DDevice9* );
        using SetMaterial = HRESULT( APIENTRY* )( IDirect3DDevice9*, const D3DMATERIAL9* );
        using SetRenderState = HRESULT( APIENTRY* )( IDirect3DDevice9*, D3DRENDERSTATETYPE, DWORD );
        using SetDepthStencilSurface = HRESULT( APIENTRY* )( IDirect3DDevice9*, IDirect3DSurface9 * );
        using SetViewport = HRESULT( APIENTRY* )( IDirect3DDevice9*, D3DVIEWPORT9 * );
    }

    enum VTable
    {
        SetDepthStencilSurface      = 39,
        BeginScene                  = 41,
        EndScene                    = 42,
        SetViewport                 = 47,
        SetMaterial                 = 49,
        SetRenderState              = 57,
    };

    /*
        QueryInterface // 0
        AddRef // 1
        Release // 2
        TestCooperativeLevel // 3
        GetAvailableTextureMem // 4
        EvictManagedResources // 5
        GetDirect3D // 6
        GetDeviceCaps // 7
        GetDisplayMode // 8
        GetCreationParameters // 9
        SetCursorProperties // 10
        SetCursorPosition // 11
        ShowCursor // 12
        CreateAdditionalSwapChain // 13
        GetSwapChain // 14
        GetNumberOfSwapChains // 15
        Reset // 16
        Present // 17
        GetBackBuffer // 18
        GetRasterStatus // 19
        SetDialogBoxMode // 20
        SetGammaRamp // 21
        GetGammaRamp // 22
        CreateTexture // 23
        CreateVolumeTexture // 24
        CreateCubeTexture // 25
        CreateVertexBuffer // 26
        CreateIndexBuffer // 27
        CreateRenderTarget // 28
        CreateDepthStencilSurface // 29
        UpdateSurface // 30
        UpdateTexture // 31
        GetRenderTargetData // 32
        GetFrontBufferData // 33
        StretchRect // 34
        ColorFill // 35
        CreateOffscreenPlainSurface // 36
        SetRenderTarget // 37
        GetRenderTarget // 38
        SetDepthStencilSurface // 39
        GetDepthStencilSurface // 40
        BeginScene // 41
        EndScene // 42
        Clear // 43
        SetTransform // 44
        GetTransform // 45
        MultiplyTransform // 46
        SetViewport // 47
        GetViewport // 48
        SetMaterial // 49
        GetMaterial // 50
        SetLight // 51
        GetLight // 52
        LightEnable // 53
        GetLightEnable // 54
        SetClipPlane // 55
        GetClipPlane // 56
        SetRenderState // 57
        GetRenderState // 58
        CreateStateBlock // 59
        BeginStateBlock // 60
        EndStateBlock // 61
        SetClipStatus // 62
        GetClipStatus // 63
        GetTexture // 64
        SetTexture // 65
        GetTextureStageState // 66
        SetTextureStageState // 67
        GetSamplerState // 68
        SetSamplerState // 69
        ValidateDevice // 70
        SetPaletteEntries // 71
        GetPaletteEntries // 72
        SetCurrentTexturePalette // 73
        GetCurrentTexturePalette // 74
        SetScissorRect // 75
        GetScissorRect // 76
        SetSoftwareVertexProcessing // 77
        GetSoftwareVertexProcessing // 78
        SetNPatchMode // 79
        GetNPatchMode // 80
        DrawPrimitive // 81
        DrawIndexedPrimitive // 82
        DrawPrimitiveUP // 83
        DrawIndexedPrimitiveUP // 84
        ProcessVertices // 85
        CreateVertexDeclaration // 86
        SetVertexDeclaration // 87
        GetVertexDeclaration // 88
        SetFVF // 89
        GetFVF // 90
        CreateVertexShader // 91
        SetVertexShader // 92
        GetVertexShader // 93
        SetVertexShaderConstantF // 94
        GetVertexShaderConstantF // 95
        SetVertexShaderConstantI // 96
        GetVertexShaderConstantI // 97
        SetVertexShaderConstantB // 98
        GetVertexShaderConstantB // 99
        SetStreamSource // 100
        GetStreamSource // 101
        SetStreamSourceFreq // 102
        GetStreamSourceFreq // 103
        SetIndices // 104
        GetIndices // 105
        CreatePixelShader // 106
        SetPixelShader // 107
        GetPixelShader // 108
        SetPixelShaderConstantF // 109
        GetPixelShaderConstantF // 110
        SetPixelShaderConstantI // 111
        GetPixelShaderConstantI // 112
        SetPixelShaderConstantB // 113
        GetPixelShaderConstantB // 114
        DrawRectPatch // 115
        DrawTriPatch // 116
        DeletePatch // 117

        CreateQuery // 118
*/

    using Color = uint32_t;

    enum Colors : uint32_t
    {
        Green			= D3DCOLOR_ARGB(255, 000, 255, 000),
        GreenAlpha      = D3DCOLOR_ARGB(100, 000, 255, 000),
        Red				= D3DCOLOR_ARGB(255, 255, 000, 000),
        Blue			= D3DCOLOR_ARGB(255, 000, 000, 255),
        Orange			= D3DCOLOR_ARGB(255, 255, 165, 000),
        Yellow			= D3DCOLOR_ARGB(255, 255, 255, 000),
        Pink			= D3DCOLOR_ARGB(255, 255, 192, 203),
        Cyan			= D3DCOLOR_ARGB(255, 000, 255, 255),
        Purple			= D3DCOLOR_ARGB(255, 160, 032, 240),
        Black			= D3DCOLOR_ARGB(255, 000, 000, 000),
        White			= D3DCOLOR_ARGB(255, 255, 255, 255),
        WhiteAlpha      = D3DCOLOR_ARGB(100, 255, 255, 255 ),
        Grey			= D3DCOLOR_ARGB(255, 112, 112, 112),
        SteelBlue		= D3DCOLOR_ARGB(255, 033, 104, 140),
        LightSteelBlue	= D3DCOLOR_ARGB(255, 201, 255, 255),
        LightBlue		= D3DCOLOR_ARGB(255, 026, 140, 306),
        Salmon			= D3DCOLOR_ARGB(255, 196, 112, 112),
        Brown			= D3DCOLOR_ARGB(255, 168,  99, 020),
        Teal			= D3DCOLOR_ARGB(255,  38, 140, 140),
        Lime			= D3DCOLOR_ARGB(255, 050, 205, 050),
        ElectricLime	= D3DCOLOR_ARGB(255, 204, 255, 000),
        Gold			= D3DCOLOR_ARGB(255, 255, 215, 000),
        OrangeRed		= D3DCOLOR_ARGB(255, 255, 69, 0)   ,
        GreenYellow		= D3DCOLOR_ARGB(255, 173, 255, 047),
        AquaMarine		= D3DCOLOR_ARGB(255, 127, 255, 212),
        SkyBlue			= D3DCOLOR_ARGB(255, 000, 191, 255),
        SlateBlue		= D3DCOLOR_ARGB(255, 132, 112, 255),
        Crimson			= D3DCOLOR_ARGB(255, 220, 020, 060),
        DarkOliveGreen	= D3DCOLOR_ARGB(255, 188, 238, 104),
        PaleGreen		= D3DCOLOR_ARGB(255, 154, 255, 154),
        DarkGoldenRod	= D3DCOLOR_ARGB(255, 255, 185, 015),
        FireBrick		= D3DCOLOR_ARGB(255, 255,  48,  48),
        DarkBlue		= D3DCOLOR_ARGB(255, 000, 000, 204),
        DarkerBlue		= D3DCOLOR_ARGB(255, 000, 000, 153),
        DarkYellow		= D3DCOLOR_ARGB(255, 255, 204, 000),
        LightYellow		= D3DCOLOR_ARGB(255, 255, 255, 153),
        DarkOutline		= D3DCOLOR_ARGB(255, 37,   48,  52),
        TBlack			= D3DCOLOR_ARGB(180, 000, 000, 000)
    };

    class Geometry
    {
    public:
        struct Vertex
        {
            Vector3f Position;
            Colors   Color;
        };

        enum Type
        {
            Line,
            Triangle
        };

        Geometry( Colors color, Type type );

        virtual void SetDeviceState( IDirect3DDevice9 * device ) const;
        virtual void Render( IDirect3DDevice9* device ) const = 0;

        void         Clear()   { m_vertices.clear(); }

    protected:
        std::vector< Vertex >   m_vertices;
        Colors                  m_color;
        Type                    m_type;
    };

    class TriangleGeometry : public Geometry
    {
    public:
        TriangleGeometry( Colors color );

        void            AddTriangle( const Vector3f & p0, const Vector3f & p1, const Vector3f & p2, std::optional< Colors > color = std::nullopt );
        virtual void    Render( IDirect3DDevice9* device ) const override;
    };

    class LineGeometry : public Geometry
    {
    public:
        LineGeometry( Colors color );

        void            AddLine( const Vector3f & p0, const Vector3f & p1, std::optional< Colors > color = std::nullopt );
        virtual void    Render( IDirect3DDevice9* device ) const override;
    };

    struct DeviceContext
    {
        using RenderStates = std::array< DWORD, D3DRS_BLENDOPALPHA + 1 >;
        using RenderTransforms = std::array< D3DXMATRIX, D3DTS_TEXTURE7 + 1 >;

        struct DeviceState
        {
            IDirect3DPixelShader9 *  ps = nullptr;
            IDirect3DVertexShader9 * vs = nullptr;
            IDirect3DBaseTexture9 *  tex0 = nullptr;
            DWORD                    fvf;
            RenderStates             states;

            RenderTransforms         transforms;
        };

        static void Store( IDirect3DDevice9* device, DeviceState & state );
        static void Restore( IDirect3DDevice9* device, const DeviceState & state );
    };

    struct Transform
    {
        D3DXMATRIX world;
        D3DXMATRIX view;
        D3DXMATRIX proj;

        static Transform FromCamera( const Wow::Camera & camera );
    };

    class Renderer
    {
    public:
        Renderer( VTABLE_TYPE* vtable );
        ~Renderer();

        void                        SetupDirectXHooks( VTABLE_TYPE* vtable );

        bool                        IsOnScreen( const Vector2f & coord );
        std::optional< Vector2f >   GetScreenCoord( Wow::Camera & camera, const Vector3f & pos );

        void                        RenderQuad( float x, float y, float width, float height, Color color = Colors::White );
        void                        RenderTriangle( const Vector2f & p0, const Vector2f & p1, const Vector2f & p2, Color color = Colors::White );
        void                        RenderLine( const Vector2f & start, const Vector2f & end, Color color = Colors::White );

        void                        RenderGeometry( const Geometry & g, std::optional< Transform* > transform = std::nullopt );
    protected:
        IDirect3DDevice9 *          m_device;
    };

    extern Renderer * GetRenderer();
}

#endif // Renderer_hpp__
