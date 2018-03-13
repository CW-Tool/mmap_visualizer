#include "LuaState.hpp"

namespace Wow
{
    LuaState::LuaState( Debugger::ProcessMemory & memory )
    {
        m_state = memory.Read< lua_State* >( LuaOffsets::I_State );

        m_doString = reinterpret_cast< luaL_dostring >( LuaOffsets::F_DoString );
        m_loadBuffer = reinterpret_cast< luaL_loadbuffer >( LuaOffsets::F_LoadBuffer );

        m_pcall = reinterpret_cast< lua_pcall >( LuaOffsets::F_PCall );
        m_getTop = reinterpret_cast< lua_gettop >( LuaOffsets::F_GetTop );
        m_setTop = reinterpret_cast< lua_settop >( LuaOffsets::F_SetTop );
        m_type = reinterpret_cast< lua_type >( LuaOffsets::F_Type );
        m_toNumber = reinterpret_cast< lua_tonumber >( LuaOffsets::F_ToNumber );
        m_toString = reinterpret_cast< lua_tostring >( LuaOffsets::F_ToString );
        m_toBoolean = reinterpret_cast< lua_toboolean >( LuaOffsets::F_ToBoolean );
    }

    void LuaState::lua_pop( lua_State * state, int n )
    {
        m_setTop( state, -( n )-1 );
    }

    LuaReturnValues LuaState::Execute( const std::string & script )
    {
        int top = m_getTop( m_state );

        int status = m_loadBuffer( m_state, script.c_str(), script.size(), "DEBUGGER" );

        //! #TODO: handle error
        if ( status != 0 )
            return std::nullopt;

        status = m_pcall( m_state, 0, -1, 0 );

        //! #TODO: handle error
        if ( status != 0 )
            return std::nullopt;

        //!#TODO: handle return values
        int returnCount = m_getTop( m_state ) - top;

        LuaReturnValues result;
        
        auto & data = result.emplace();
        for ( int idx = 0; idx < returnCount; ++idx )
        {
            LuaType type = static_cast< LuaType >( m_type( m_state, -1 ) );
            switch ( type )
            {
                case LuaType::LUA_TBOOLEAN:
                {
                    data.push_back( m_toBoolean( m_state, -1 ) != 0 );
                    break;
                }
                case LuaType::LUA_TNUMBER:
                {
                    data.push_back( m_toNumber( m_state, -1 ) );
                    break;
                }
                case LuaType::LUA_TSTRING:
                {
                    data.push_back( m_toString( m_state, -1 ) );
                    break;
                }
                default:
                    break;
            }

            lua_pop( m_state, 1 );
        }

        //m_setTop( m_state, top );
        return result;
    }

    LuaRootFrame * LuaState::CreateFrame( const std::string & name )
    {
        auto frame = std::make_unique< LuaRootFrame >( *this, name );
        m_frames.push_back( std::move( frame ) );

        return m_frames.back().get();
    }
}
