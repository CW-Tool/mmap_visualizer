#include "Renderer.hpp"
#include "Debugger.hpp"

#include "MinHook.h"


#include "wow_client/Camera.hpp"
#include "wow_client/Location.hpp"

#include <exception>
#include <iostream>

namespace Debugger
{
    Renderer * Renderer::s_renderer = nullptr;

    Renderer::Renderer( VTABLE_TYPE* vtable, Debugger * debugger )
        : m_debugger( debugger )
    {
        s_renderer = this;

        SetupDirectXHooks( vtable );
    }

    Renderer::~Renderer()
    {
        MH_Uninitialize();
    }

    void Renderer::EndScene()
    {
        m_debugger->Update();

        D3DRECT rect{ 20, 20, 100, 100 };
        D3DCOLOR color = D3DCOLOR_ARGB( 255, 255, 255, 255 );

        m_device->Clear( 1, &rect, D3DCLEAR_TARGET, color, 0, 0 );
    }

#define M_DEG2RAD 0.0174532925

    std::optional< Vector2f > Renderer::GetScreenCoord( Wow::Camera & camera, Wow::Location & location )
    {
        D3DVIEWPORT9 viewport;
        m_device->GetViewport( &viewport );

        RECT rc = { viewport.X, viewport.Y, viewport.Width, viewport.Height };

        float fDiff[ 3 ];
        fDiff[ 0 ] = location.x - camera.Position.x;
        fDiff[ 1 ] = location.y - camera.Position.y;
        fDiff[ 2 ] = location.z - camera.Position.z;

        float fProd = fDiff[ 0 ] * camera.ViewMatrix[ 0 ][ 0 ] + fDiff[ 1 ] * camera.ViewMatrix[ 0 ][ 1 ] + fDiff[ 2 ] * camera.ViewMatrix[ 0 ][ 2 ];
        if ( fProd < 0 )
            return std::nullopt;

        float fInv[ 3 ][ 3 ];
        fInv[ 0 ][ 0 ] = camera.ViewMatrix[ 1 ][ 1 ] * camera.ViewMatrix[ 2 ][ 2 ] - camera.ViewMatrix[ 1 ][ 2 ] * camera.ViewMatrix[ 2 ][ 1 ];
        fInv[ 1 ][ 0 ] = camera.ViewMatrix[ 1 ][ 2 ] * camera.ViewMatrix[ 2 ][ 0 ] - camera.ViewMatrix[ 1 ][ 0 ] * camera.ViewMatrix[ 2 ][ 2 ];
        fInv[ 2 ][ 0 ] = camera.ViewMatrix[ 1 ][ 0 ] * camera.ViewMatrix[ 2 ][ 1 ] - camera.ViewMatrix[ 1 ][ 1 ] * camera.ViewMatrix[ 2 ][ 0 ];

        float fDet = camera.ViewMatrix[ 0 ][ 0 ] * fInv[ 0 ][ 0 ] + camera.ViewMatrix[ 0 ][ 1 ] * fInv[ 1 ][ 0 ] + camera.ViewMatrix[ 0 ][ 2 ] * fInv[ 2 ][ 0 ];
        float fInvDet = 1.0f / fDet;

        fInv[ 0 ][ 1 ] = camera.ViewMatrix[ 0 ][ 2 ] * camera.ViewMatrix[ 2 ][ 1 ] - camera.ViewMatrix[ 0 ][ 1 ] * camera.ViewMatrix[ 2 ][ 2 ];
        fInv[ 0 ][ 2 ] = camera.ViewMatrix[ 0 ][ 1 ] * camera.ViewMatrix[ 1 ][ 2 ] - camera.ViewMatrix[ 0 ][ 2 ] * camera.ViewMatrix[ 1 ][ 1 ];
        fInv[ 1 ][ 1 ] = camera.ViewMatrix[ 0 ][ 0 ] * camera.ViewMatrix[ 2 ][ 2 ] - camera.ViewMatrix[ 0 ][ 2 ] * camera.ViewMatrix[ 2 ][ 0 ];
        fInv[ 1 ][ 2 ] = camera.ViewMatrix[ 0 ][ 2 ] * camera.ViewMatrix[ 1 ][ 0 ] - camera.ViewMatrix[ 0 ][ 0 ] * camera.ViewMatrix[ 1 ][ 2 ];
        fInv[ 2 ][ 1 ] = camera.ViewMatrix[ 0 ][ 1 ] * camera.ViewMatrix[ 2 ][ 0 ] - camera.ViewMatrix[ 0 ][ 0 ] * camera.ViewMatrix[ 2 ][ 1 ];
        fInv[ 2 ][ 2 ] = camera.ViewMatrix[ 0 ][ 0 ] * camera.ViewMatrix[ 1 ][ 1 ] - camera.ViewMatrix[ 0 ][ 1 ] * camera.ViewMatrix[ 1 ][ 0 ];

        //camera.ViewMatrix[ 0 ][ 0 ] = fInv[ 0 ][ 0 ] * fInvDet;
        //camera.ViewMatrix[ 0 ][ 1 ] = fInv[ 0 ][ 1 ] * fInvDet;
        //camera.ViewMatrix[ 0 ][ 2 ] = fInv[ 0 ][ 2 ] * fInvDet;
        //camera.ViewMatrix[ 1 ][ 0 ] = fInv[ 1 ][ 0 ] * fInvDet;
        //camera.ViewMatrix[ 1 ][ 1 ] = fInv[ 1 ][ 1 ] * fInvDet;
        //camera.ViewMatrix[ 1 ][ 2 ] = fInv[ 1 ][ 2 ] * fInvDet;
        //camera.ViewMatrix[ 2 ][ 0 ] = fInv[ 2 ][ 0 ] * fInvDet;
        //camera.ViewMatrix[ 2 ][ 1 ] = fInv[ 2 ][ 1 ] * fInvDet;
        //camera.ViewMatrix[ 2 ][ 2 ] = fInv[ 2 ][ 2 ] * fInvDet;

        float fView[ 3 ];
        fView[ 0 ] = fInv[ 0 ][ 0 ] * fDiff[ 0 ] + fInv[ 1 ][ 0 ] * fDiff[ 1 ] + fInv[ 2 ][ 0 ] * fDiff[ 2 ];
        fView[ 1 ] = fInv[ 0 ][ 1 ] * fDiff[ 0 ] + fInv[ 1 ][ 1 ] * fDiff[ 1 ] + fInv[ 2 ][ 1 ] * fDiff[ 2 ];
        fView[ 2 ] = fInv[ 0 ][ 2 ] * fDiff[ 0 ] + fInv[ 1 ][ 2 ] * fDiff[ 1 ] + fInv[ 2 ][ 2 ] * fDiff[ 2 ];

        float fCam[ 3 ];
        fCam[ 0 ] = -fView[ 1 ];
        fCam[ 1 ] = -fView[ 2 ];
        fCam[ 2 ] = fView[ 0 ];

        float    fScreenX = ( rc.right - rc.left ) / 2.0f;
        float    fScreenY = ( rc.bottom - rc.top ) / 2.0f;

        // Thanks pat0! Aspect ratio fix
        float    fTmpX = fScreenX / tan( ( ( camera.FieldOfView*44.0f ) / 2.0f )*M_DEG2RAD );
        float    fTmpY = fScreenY / tan( ( ( camera.FieldOfView*35.0f ) / 2.0f )*M_DEG2RAD );

        POINT pctMouse;
        //pctMouse.x = fScreenX + vCam.fX*fTmpX/vCam.fZ;
        pctMouse.x = fScreenX + fCam[ 0 ] * fTmpX / fCam[ 2 ];
        //pctMouse.y = fScreenY + vCam.fY*fTmpY/vCam.fZ;
        pctMouse.y = fScreenY + fCam[ 1 ] * fTmpY / fCam[ 2 ];

        if ( pctMouse.x < 0 || pctMouse.y < 0 || pctMouse.x > rc.right || pctMouse.y > rc.bottom )
            return std::nullopt;

        return Vector2f{ ( float )pctMouse.x, ( float )pctMouse.y };
    }

    void Renderer::RenderQuad( float x, float y, float width, float height )
    {
        const DWORD D3D_FVF = ( D3DFVF_XYZRHW | D3DFVF_DIFFUSE );

        struct Vertex
        {
            float x, y, z, ht;
            DWORD vcolor;
        };

        auto color = D3DCOLOR_ARGB( 255, 255, 255, 255 );

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

    void Renderer::RenderLine( Vector2f start, Vector2f end )
    {
        HRESULT hRet;

        const DWORD D3D_FVF = D3DFVF_XYZRHW | D3DFVF_DIFFUSE | D3DFVF_TEX1;

        struct Vertex
        {
            float x, y, z, ht;
            DWORD vcolor;
        };

        auto color = D3DCOLOR_ARGB( 255, 255, 255, 255 );

        Vertex verts[ 2 ] =
        {
            { start.x, start.y, 0.0f, 1.0f, color },
            { end.x, end.y, 0.0f, 1.0f, color },
        };

        m_device->SetPixelShader( 0 ); //fix black color
        m_device->SetRenderState( D3DRS_ALPHABLENDENABLE, true );
        m_device->SetFVF( D3D_FVF );
        m_device->SetTexture( 0, NULL );

        auto result = m_device->DrawPrimitiveUP( D3DPT_LINELIST, 2, verts, sizeof( Vertex ) );
        if ( FAILED( result ) )
        {
            std::cerr << "Failed!";
        }
    }

    void Renderer::SetupDirectXHooks( VTABLE_TYPE* vtable )
    {
        MH_STATUS status = MH_Initialize();
        if ( status != MH_OK )
        {
            std::terminate();
        }

        //! EndScene
        {
            static EndSceneFunc s_endSceneOrig = reinterpret_cast< EndSceneFunc >( vtable[ VTable::EndScene ] );
            static EndSceneFunc s_endSceneHook = []( IDirect3DDevice9* pDevice ) -> HRESULT
            {
                s_renderer->m_device = pDevice;

                s_renderer->EndScene();
                return s_endSceneOrig( pDevice );
            };

            status = MH_CreateHook( reinterpret_cast< DWORD_PTR* >( vtable[ VTable::EndScene ] ), s_endSceneHook, reinterpret_cast< void** >( &s_endSceneOrig ) );
            if ( status != MH_OK )
            {
                std::terminate();
            }

            status = MH_EnableHook( reinterpret_cast< DWORD_PTR* >( vtable[ VTable::EndScene ] ) );
            if ( status != MH_OK )
            {
                std::terminate();
            }
        }
    }
}
