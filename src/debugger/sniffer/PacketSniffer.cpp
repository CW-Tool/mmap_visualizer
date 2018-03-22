#include "PacketSniffer.hpp"

#include "Debugger.hpp"

namespace Debugger
{
    PacketSniffer * g_sniffer = nullptr;

    Opcodes PacketReader::ReadOpcode()
    {
        return Opcodes( Read< uint16_t >() );
    }

    uint64_t PacketReader::ReadPackGuid()
    {
        uint64_t guid = 0;

        uint8_t guidmark = 0;
        guidmark = Read< uint8_t >();

        for ( int i = 0; i < 8; ++i )
        {
            if ( guidmark & ( uint8_t( 1 ) << i ) )
            {
                uint8_t bit = Read< uint8_t >();
                guid |= ( uint64_t( bit ) << ( i * 8 ) );
            }
        }

        return guid;
    }

    Vector3f PacketReader::ReadPackXYZ( const Vector3f & origin )
    {
        uint32_t packed = Read<uint32_t>();

        float x = ( ( packed >> 00 ) & 0x7FF ) * 0.25f;
        float y = ( ( packed >> 11 ) & 0x7FF ) * 0.25f;
        float z = ( ( packed >> 22 ) & 0x3FF ) * 0.25f;

        Vector3f point;
        point.x = origin.x - x;
        point.y = origin.y - y;
        point.z = origin.z - z;
        return point;
    }

    using NetClient_ProcessMessage = int( __fastcall * )( void* thisPTR, void* dummy, void* param1, ServerPacket* dataStore, void* connectionId );

    PacketSniffer::PacketSniffer()
    {
        g_sniffer = this;

        RegisterHook< NetClient_ProcessMessage >( 0x631FE0, []( void* thisPTR, void* dummy, void* param1, ServerPacket* packet, void* connectionId )
        {
            PacketReader reader( *packet );

            Opcodes opcode = reader.ReadOpcode();
            for ( auto && handler : g_sniffer->m_handlers[ opcode ] )
            {
                handler( reader );
            }

            return 0;
        } );
    }

    PacketSniffer * GetSniffer()
    {
        return g_sniffer;
    }
}
