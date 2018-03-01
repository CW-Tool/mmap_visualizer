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

    void LuaState::Execute( const std::string & script )
    {
        int top = m_getTop( m_state );

        int status = m_loadBuffer( m_state, script.c_str(), script.size(), "DEBUGGER" );

        //! #TODO: handle error
        if ( status != 0 )
            return;

        status = m_pcall( m_state, 0, 0, 0 );

        //! #TODO: handle error
        if ( status != 0 )
            return;

        //!#TODO: handle return values
        int returnCount = m_getTop( m_state ) - top;
        m_setTop( m_state, top );
    }
}
