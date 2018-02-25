#include "Renderer.hpp"

#include "MinHook.h"

#include <exception>
#include <iostream>

namespace Debugger
{
    Renderer * Renderer::s_renderer = nullptr;

    Renderer::Renderer( VTABLE_TYPE* vtable )
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
        D3DRECT rect{ 20, 20, 100, 100 };
        D3DCOLOR color = D3DCOLOR_ARGB( 255, 255, 255, 255 );

        m_device->Clear( 1, &rect, D3DCLEAR_TARGET, color, 0, 0 );
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
