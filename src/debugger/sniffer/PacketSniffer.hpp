#ifndef PacketSniffer_hpp__
#define PacketSniffer_hpp__

#include "math/Vector.hpp"

#include <functional>
#include <wtypes.h>

namespace Debugger
{
    enum class Opcodes
    {
        SMSG_MONSTER_MOVE           = 0x0DD,
        SMSG_MONSTER_MOVE_TRANSPORT = 0x2AE,
    };

    struct ServerPacket
    {
        void* vTable;
        BYTE* buffer;
        DWORD base;
        DWORD alloc;
        DWORD size;
        DWORD read;
    };

    struct PacketReader
    {
    public:
        PacketReader( ServerPacket & p )
            : m_buffer( p.buffer )
            , m_carret( 0u )
            , m_size( p.size )
        {
            m_opcode = ReadOpcode();
        }

        Opcodes GetOpcode() const
        {
            return m_opcode;
        }

        uint64_t    ReadPackGuid();
        Vector3f    ReadPackXYZ( const Vector3f & origin );

        template< typename T >
        T           Read()
        {
            if ( m_size < m_carret + sizeof( T ) )
                throw std::runtime_error("out of bounds");

            T result = *reinterpret_cast< T* >( &m_buffer[ m_carret ] );
            m_carret += sizeof( T );
            return result;
        }

    protected:
        Opcodes     ReadOpcode();

        Opcodes     m_opcode;

        size_t      m_carret;
        uint8_t *   m_buffer;
        size_t      m_size;
    };

    using PacketHandler = std::function< void( PacketReader & ) >;

    class PacketSniffer
    {
    public:
        PacketSniffer();

        void AddPacketListener( Opcodes opcode, PacketHandler handler )
        {
            m_handlers[ opcode ].push_back( std::move( handler ) );
        }

    protected:
        using PacketHandlerVec = std::vector< PacketHandler >;

        std::unordered_map< Opcodes, PacketHandlerVec > m_handlers;
    };

    extern PacketSniffer * GetSniffer();
}

#endif // PacketSniffer_hpp__
