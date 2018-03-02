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
        : m_customRenderInProgress( false )
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

    std::optional< Vector2f > Renderer::GetScreenCoord( Wow::Camera & camera, Wow::Location & location )
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

    void Renderer::StartCustomRender()
    {
        if ( m_customRenderInProgress )
        {
            std::terminate();
        }

        m_customRenderInProgress = true;

        m_device->GetTexture( 0, &m_state.texture );
        m_device->GetPixelShader( &m_state.pixelShader );
        m_device->GetRenderState( D3DRS_ALPHABLENDENABLE, &m_state.alphaBlend );
        m_device->GetFVF( &m_state.fvf );
        m_device->GetTransform( D3DTS_VIEW, &m_state.view );
        m_device->GetTransform( D3DTS_PROJECTION, &m_state.proj );
        m_device->GetTransform( D3DTS_WORLD, &m_state.world );
        m_device->GetDepthStencilSurface( &m_state.depthStencil );
    }

    void Renderer::EndCustomRender()
    {
        if ( !m_customRenderInProgress )
        {
            std::terminate();
        }

        m_device->SetTexture( 0, m_state.texture );
        m_device->SetPixelShader( m_state.pixelShader );
        m_device->SetRenderState( D3DRS_ALPHABLENDENABLE, m_state.alphaBlend );
        m_device->SetFVF( m_state.fvf );

        m_device->SetTransform( D3DTS_VIEW, &m_state.view );
        m_device->SetTransform( D3DTS_PROJECTION, &m_state.proj );
        m_device->SetTransform( D3DTS_WORLD, &m_state.world );
        m_device->SetDepthStencilSurface( m_state.depthStencil );

        m_customRenderInProgress = false;
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

        auto status = MH_CreateHook( vtable_ptr, hookBefore ? s_RealHookBefore : s_RealHookAfter , reinterpret_cast< void** >( &s_OrigFunc ) );
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

    struct CustomRenderState
    {
        uint8_t             zFuncChangeCount = 0;
        IDirect3DSurface9*  depthSurface = nullptr;

    };

    CustomRenderState g_state;

    void Renderer::SetupDirectXHooks( VTABLE_TYPE* vtable )
    {
        CreateD3D9Hook< D3D9::BeginScene, VTable::BeginScene >( vtable, []( IDirect3DDevice9* pDevice ) -> HRESULT
        {
            GetRenderer()->m_device = pDevice;

            g_state.zFuncChangeCount = 0;
            g_state.depthSurface = nullptr;

            return S_OK;
        } );

        CreateD3D9Hook< D3D9::EndScene, VTable::EndScene >( vtable, []( IDirect3DDevice9* pDevice ) -> HRESULT
        {
            auto debugger = GetDebugger();
            if ( debugger != nullptr )
            {
                debugger->Update();

                pDevice->SetRenderState( D3DRS_ZFUNC, D3DCMP_ALWAYS );
                pDevice->SetDepthStencilSurface( g_state.depthSurface );

                debugger->RenderNavMesh();
            }

            return S_OK;
        } );

        CreateD3D9Hook< D3D9::SetDepthStencilSurface, VTable::SetDepthStencilSurface >( vtable, []( IDirect3DDevice9* pDevice, IDirect3DSurface9* surface ) -> HRESULT
        {
            if ( g_state.zFuncChangeCount == 2 )
            {
                g_state.depthSurface = surface;
            }

            return S_OK;
        } );

        CreateD3D9Hook< D3D9::SetRenderState, VTable::SetRenderState >( vtable, []( IDirect3DDevice9* pDevice, D3DRENDERSTATETYPE type, DWORD value ) -> HRESULT
        {
            switch ( type )
            {
                case D3DRS_ZFUNC:
                {
                    ++g_state.zFuncChangeCount;
                    break;
                }
            }

            return S_OK;
        } );
    }

    extern Renderer * GetRenderer()
    {
        return g_renderer;
    }
}
