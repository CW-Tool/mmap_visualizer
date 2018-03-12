#include "Renderer.hpp"
#include "Debugger.hpp"

#include "MinHook.h"

#include "wow_client/Camera.hpp"
#include "wow_client/Location.hpp"

#include <exception>
#include <iostream>

#include "d3dx9.h"

#if defined _M_X64
#pragma comment(lib, "x64/d3dx9.lib") 
#elif defined _M_IX86
#pragma comment(lib, "x86/d3dx9.lib")
#endif

namespace Debugger
{
    Renderer * g_renderer = nullptr;

    Renderer::Renderer( VTABLE_TYPE* vtable )
    {
        g_renderer = this;

        MH_STATUS status = MH_Initialize();
        if ( status != MH_OK )
        {
            std::terminate();
        }

        SetupDirectXHooks( vtable );
    }

    Renderer::~Renderer()
    {
        MH_Uninitialize();
    }

    bool Renderer::IsOnScreen( const Vector2f & coord )
    {
        D3DVIEWPORT9 viewport;
        m_device->GetViewport( &viewport );

        return coord.x >= 0.0 && coord.y >= 0.0f && coord.x <= viewport.Width && coord.y <= viewport.Height;
    }

    std::optional< Vector2f > Renderer::GetScreenCoord( Wow::Camera & camera, const Vector3f & location )
    {
        D3DXVECTOR3 pos( location.x, location.y, location.z );

        D3DVIEWPORT9 viewport;
        m_device->GetViewport( &viewport );

        D3DXMATRIX proj;
        D3DXMatrixPerspectiveFovRH( &proj, camera.FieldOfView * 0.6f, camera.Aspect, camera.NearPlane, camera.FarPlane );

        auto camPos = camera.Position;
        auto camDir = camera.GetForwardDir();
        auto camUp = camera.GetUpDir();

        D3DXVECTOR3 eye( camPos.x, camPos.y, camPos.z );
        D3DXVECTOR3 at( eye.x + camDir.x, eye.y + camDir.y, eye.z + camDir.z );
        D3DXVECTOR3 up( camUp.x, camUp.y, camUp.z );

        D3DXMATRIX view;
        D3DXMatrixLookAtRH( &view, &eye, &at, &up );

        D3DXMATRIX world;
        D3DXMatrixIdentity( &world );

        D3DXVECTOR3 coord;
        D3DXVec3Project( &coord, &pos, &viewport, &proj, &view, &world );

        if ( coord.z > 1.0f || coord.x < 0.0f || coord.x > viewport.Width || coord.y < 0.0f || coord.y > viewport.Height )
            return std::nullopt;

        return Vector2f{ coord.x, coord.y };
    }

    void Renderer::RenderQuad( float x, float y, float width, float height, Color color )
    {
        const DWORD D3D_FVF = ( D3DFVF_XYZRHW | D3DFVF_DIFFUSE );

        struct Vertex
        {
            float x, y, z, ht;
            DWORD vcolor;
        };

        Vertex verts[ 4 ] =
        {
            { x, ( y + height ), 0.0f, 0.0f, color },
            { x, y, 0.0f, 0.0f, color },
            { ( x + width ), ( y + height ), 0.0f, 0.0f, color },
            { ( x + width ), y, 0.0f, 0.0f, color }
        };

        m_device->SetPixelShader( 0 );
        m_device->SetRenderState( D3DRS_ALPHABLENDENABLE, true );
        m_device->SetFVF( D3D_FVF );
        m_device->SetTexture( 0, NULL );

        auto result = m_device->DrawPrimitiveUP( D3DPT_TRIANGLESTRIP, 2, verts, sizeof( Vertex ) );
        if ( FAILED( result ) )
        {
            std::cerr << "Failed!";
        }
    }

    void Renderer::RenderTriangle( const Vector2f & p0, const Vector2f & p1, const Vector2f & p2, Color color )
    {
        const DWORD D3D_FVF = ( D3DFVF_XYZRHW | D3DFVF_DIFFUSE );

        struct Vertex
        {
            float x, y, z, ht;
            DWORD vcolor;
        };

        Vertex verts[ 4 ] =
        {
            { p0.x, p0.y, 0.0f, 1.0f, color },
            { p1.x, p1.y, 0.0f, 1.0f, color },
            { p2.x, p2.y, 0.0f, 1.0f, color },
        };

        m_device->SetPixelShader( 0 );
        m_device->SetRenderState( D3DRS_ALPHABLENDENABLE, true );
        m_device->SetFVF( D3D_FVF );
        m_device->SetTexture( 0, NULL );

        auto result = m_device->DrawPrimitiveUP( D3DPT_TRIANGLELIST, 1, verts, sizeof( Vertex ) );
        if ( FAILED( result ) )
        {
            std::cerr << "Failed!";
        }
    }

    void Renderer::RenderLine( const Vector2f & start, const Vector2f & end, Color color )
    {
        const DWORD D3D_FVF = D3DFVF_XYZRHW | D3DFVF_DIFFUSE | D3DFVF_TEX1;

        struct Vertex
        {
            float x, y, z, ht;
            DWORD vcolor;
        };

        Vertex verts[ 2 ] =
        {
            { start.x, start.y, 0.0f, 1.0f, color },
            { end.x, end.y, 0.0f, 1.0f, color },
        };

        m_device->SetPixelShader( 0 ); //fix black color
        m_device->SetRenderState( D3DRS_ALPHABLENDENABLE, true );
        m_device->SetFVF( D3D_FVF );
        m_device->SetTexture( 0, NULL );

        auto result = m_device->DrawPrimitiveUP( D3DPT_LINELIST, 1, verts, sizeof( Vertex ) );
        if ( FAILED( result ) )
        {
            std::cerr << "Failed!";
        }
    }

    void Renderer::RenderGeometry( const Geometry & g, std::optional< Transform* > transform )
    {
        static DeviceContext::DeviceState s_state;
        DeviceContext::Store( m_device, s_state );

        if ( transform )
        {

            m_device->SetTransform( D3DTS_WORLD, &( *transform )->world );
            m_device->SetTransform( D3DTS_VIEW, &( *transform )->view );
            m_device->SetTransform( D3DTS_PROJECTION, &( *transform )->proj );
        }

        g.Render( m_device );

        DeviceContext::Restore( m_device, s_state );
    }

    template< class T, int VTABLE_IDX >
    void CreateD3D9Hook( VTABLE_TYPE* vtable, T && hook, bool hookBefore = true )
    {
        auto vtable_ptr = reinterpret_cast< DWORD_PTR* >( vtable[ VTABLE_IDX ] );

        static T s_OrigFunc = reinterpret_cast< T >( vtable_ptr );
        static T s_HookFunc = std::move( hook );

        static T s_RealHookBefore = []( auto ... args )
        {
            s_HookFunc( args... );

            return s_OrigFunc( args... );
        };

        static T s_RealHookAfter = []( auto ... args )
        {
            auto result = s_OrigFunc( args... );

            s_HookFunc( args... );

            return result;
        };

        auto status = MH_CreateHook( vtable_ptr, hookBefore ? s_RealHookBefore : s_RealHookAfter, reinterpret_cast< void** >( &s_OrigFunc ) );
        if ( status != MH_OK )
        {
            std::terminate();
        }

        status = MH_EnableHook( vtable_ptr );
        if ( status != MH_OK )
        {
            std::terminate();
        }
    };

    uint8_t g_renderNavMesh = 0u;

    void Renderer::SetupDirectXHooks( VTABLE_TYPE* vtable )
    {
        CreateD3D9Hook< D3D9::BeginScene, VTable::BeginScene >( vtable, []( IDirect3DDevice9* pDevice ) -> HRESULT
        {
            g_renderNavMesh = 0u;
            GetRenderer()->m_device = pDevice;

            auto debugger = GetDebugger();
            if ( debugger != nullptr )
            {
                debugger->Update();
            }

            return S_OK;
        } );

        CreateD3D9Hook< D3D9::SetRenderState, VTable::SetRenderState >( vtable, []( IDirect3DDevice9*, auto type, auto value )
        {
            const uint8_t STATE_COUNTER = 1;

            if ( type == D3DRS_LIGHTING && value == FALSE )
            {
                ++g_renderNavMesh;

                auto debugger = GetDebugger();
                if ( g_renderNavMesh == STATE_COUNTER && debugger != nullptr )
                {
                    debugger->Render();
                }
            }

            return S_OK;
        } );
    }

    extern Renderer * GetRenderer()
    {
        return g_renderer;
    }

    Geometry::Geometry( Colors color, Type type )
        : m_color( color )
        , m_type( type )
    {

    }

    void Geometry::SetDeviceState( IDirect3DDevice9 * device ) const
    {
        device->SetPixelShader( nullptr );
        device->SetVertexShader( nullptr );
        device->SetTexture( 0, nullptr );

        device->SetRenderState( D3DRS_ZWRITEENABLE, FALSE );
        device->SetRenderState( D3DRS_ZFUNC, D3DCMP_LESSEQUAL );
        device->SetRenderState( D3DRS_STENCILENABLE, FALSE );

        device->SetRenderState( D3DRS_ALPHATESTENABLE, FALSE );
        device->SetRenderState( D3DRS_FOGENABLE, TRUE );
        device->SetRenderState( D3DRS_LIGHTING, FALSE );
        device->SetRenderState( D3DRS_AMBIENT, 0x0000 );

        device->SetRenderState( D3DRS_COLORVERTEX, TRUE );
        device->SetRenderState( D3DRS_DIFFUSEMATERIALSOURCE, D3DMCS_COLOR1 );
        device->SetRenderState( D3DRS_SPECULARMATERIALSOURCE, D3DMCS_COLOR1 );
        device->SetRenderState( D3DRS_AMBIENTMATERIALSOURCE, D3DMCS_COLOR1 );
        device->SetRenderState( D3DRS_EMISSIVEMATERIALSOURCE, D3DMCS_COLOR1 );

        device->SetRenderState( D3DRS_ALPHABLENDENABLE, TRUE );
        device->SetRenderState( D3DRS_SEPARATEALPHABLENDENABLE, FALSE );
        device->SetRenderState( D3DRS_BLENDOP, D3DBLENDOP_ADD );
        device->SetRenderState( D3DRS_SRCBLEND, D3DBLEND_SRCALPHA );
        device->SetRenderState( D3DRS_DESTBLEND, D3DBLEND_INVSRCALPHA );
        device->SetRenderState( D3DRS_TEXTUREFACTOR, 0xFFFFFFFF );
        device->SetRenderState( D3DRS_COLORWRITEENABLE, D3DCOLORWRITEENABLE_ALPHA | D3DCOLORWRITEENABLE_BLUE | D3DCOLORWRITEENABLE_GREEN | D3DCOLORWRITEENABLE_RED );
        device->SetRenderState( D3DRS_ANTIALIASEDLINEENABLE, TRUE );
        device->SetRenderState( D3DRS_BLENDFACTOR, 0xffffffff );
    }

    TriangleGeometry::TriangleGeometry( Colors color )
        : Geometry( color, Type::Triangle )
    {
        m_vertices.reserve( 200 );
    }

    void TriangleGeometry::AddTriangle( const Vector3f & p0, const Vector3f & p1, const Vector3f & p2, std::optional< Colors > color )
    {
        m_vertices.push_back( Vertex{ p0, color ? *color : m_color } );
        m_vertices.push_back( Vertex{ p1, color ? *color : m_color } );
        m_vertices.push_back( Vertex{ p2, color ? *color : m_color } );
    }

    void TriangleGeometry::Render( IDirect3DDevice9* device ) const
    {
        SetDeviceState( device );

        device->SetFVF( D3DFVF_XYZ | D3DFVF_DIFFUSE );
        device->DrawPrimitiveUP( D3DPT_TRIANGLELIST, m_vertices.size() / 3, m_vertices.data(), sizeof( Vertex ) );
    }

    LineGeometry::LineGeometry( Colors color )
        : Geometry( color, Type::Line )
    {
        m_vertices.reserve( 200 );
    }

    void LineGeometry::AddLine( const Vector3f & p0, const Vector3f & p1, std::optional< Colors > color )
    {
        m_vertices.push_back( Vertex{ p0, color ? *color : m_color } );
        m_vertices.push_back( Vertex{ p1, color ? *color : m_color } );
    }

    void LineGeometry::Render( IDirect3DDevice9* device ) const
    {
        SetDeviceState( device );

        device->SetFVF( D3DFVF_XYZ | D3DFVF_DIFFUSE );
        device->DrawPrimitiveUP( D3DPT_LINELIST, m_vertices.size() / 2, m_vertices.data(), sizeof( Vertex ) );
    }

    void DeviceContext::Store( IDirect3DDevice9* device, DeviceState & state )
    {
        device->GetPixelShader( &state.ps );
        device->GetVertexShader( &state.vs );
        device->GetTexture( 0, &state.tex0 );

        for ( auto idx = 0; idx < state.states.size(); ++idx )
        {
            device->GetRenderState( D3DRENDERSTATETYPE( idx ), &state.states[ idx ] );
        }

        for ( auto idx = 0; idx < state.transforms.size(); ++idx )
        {
            device->GetTransform( D3DTRANSFORMSTATETYPE( idx ), &state.transforms[ idx ] );
        }

        device->GetFVF( &state.fvf );
    }

    void DeviceContext::Restore( IDirect3DDevice9* device, const DeviceState & state )
    {
        device->SetPixelShader( state.ps );
        device->SetVertexShader( state.vs );
        device->SetTexture( 0, state.tex0 );

        for ( auto idx = 0; idx < state.states.size(); ++idx )
        {
            device->SetRenderState( D3DRENDERSTATETYPE( idx ), state.states[ idx ] );
        }

        for ( auto idx = 0; idx < state.transforms.size(); ++idx )
        {
            device->SetTransform( D3DTRANSFORMSTATETYPE( idx ), &state.transforms[ idx ] );
        }

        device->SetFVF( state.fvf );
    }

    Transform Transform::FromCamera( const Wow::Camera & camera )
    {
        Transform result;

        D3DXMatrixPerspectiveFovRH( &result.proj, camera.FieldOfView * 0.6f, camera.Aspect, camera.NearPlane, camera.FarPlane );

        auto camPos = camera.Position;
        auto camDir = camera.GetForwardDir();
        auto camUp = camera.GetUpDir();

        D3DXVECTOR3 eye( camPos.x, camPos.y, camPos.z );
        D3DXVECTOR3 at( eye.x + camDir.x, eye.y + camDir.y, eye.z + camDir.z );
        D3DXVECTOR3 up( camUp.x, camUp.y, camUp.z );

        D3DXMatrixLookAtRH( &result.view, &eye, &at, &up );
        D3DXMatrixIdentity( &result.world );

        return result;
    }
}
