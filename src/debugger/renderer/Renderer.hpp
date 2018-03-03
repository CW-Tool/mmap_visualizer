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
    }

    enum VTable
    {
        SetDepthStencilSurface      = 39,
        BeginScene                  = 41,
        EndScene                    = 42,
        SetMaterial                 = 49,
        SetRenderState              = 57,
    };

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

        void         AddLine( const Vector3f & p0, const Vector3f & p1, std::optional< Colors > color = std::nullopt );
        virtual void Render( IDirect3DDevice9* device ) const override;
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

        void                        RenderGeometry( const Geometry & g, std::optional< Wow::Camera > camera = std::nullopt );
    protected:
        IDirect3DDevice9 *          m_device;
    };

    extern Renderer * GetRenderer();
}

#endif // Renderer_hpp__
