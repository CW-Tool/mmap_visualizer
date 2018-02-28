#ifndef LuaState_hpp__
#define LuaState_hpp__

#include "memory/ProcessMemory.hpp"

#include <string>

namespace Wow
{
    enum LuaOffsets
    {
        I_State        = 0x00D3F78C,
        F_DoString     = 0x00819210,
        F_LoadBuffer   = 0x0084F860,
        F_PCall        = 0x0084EC50,
        F_GetTop       = 0x0084DBD0,
        F_SetTop       = 0x0084DBF0,
        F_Type         = 0x0084DEB0,
        F_ToNumber     = 0x0084E030,
        F_ToString     = 0x0084E0E0,
        F_ToBoolean    = 0x0084E0B0,
    };

    enum LuaType
    {
        LUA_TNONE           = -1,
        LUA_TNIL            = 0,
        LUA_TBOOLEAN        = 1,
        LUA_TLIGHTUSERDATA  = 2,
        LUA_TNUMBER         = 3,
        LUA_TSTRING         = 4,
        LUA_TTABLE          = 5,
        LUA_TFUNCTION       = 6,
        LUA_TUSERDATA       = 7,
        LUA_TTHREAD         = 8,
    };

    struct lua_State {};

    using luaL_loadbuffer = int( __cdecl * )( lua_State* L, const char *buff, size_t sz, const char *name );
    using luaL_dostring   = int( __cdecl * )( lua_State* L, const char *str );

    using lua_pcall       = int( __cdecl * )( lua_State *L, int nargs, int nresults, int errfunc );
    using lua_gettop      = int( __cdecl * )( lua_State* L );
    using lua_settop      = void( __cdecl * )( lua_State* L, int index );
    using lua_tostring    = const char *( __cdecl * )( lua_State *L, int index );
    using lua_tonumber    = double ( __cdecl * )( lua_State *L, int index );
    using lua_toboolean   = int( __cdecl * )( lua_State *L, int index );
    using lua_type        = int( __cdecl * )( lua_State *L, int index );

    class LuaState
    {
    public:
        LuaState( Debugger::ProcessMemory & memory );

        void Execute( const std::string & script );

    protected:
        lua_State*      m_state;

        luaL_loadbuffer m_loadBuffer;
        luaL_dostring   m_doString;

        lua_gettop      m_getTop;
        lua_settop      m_setTop;
        lua_pcall       m_pcall;
        lua_tostring    m_toString;
        lua_tonumber    m_toNumber;
        lua_toboolean   m_toBoolean;
        lua_type        m_type;
    };
}

#endif // LuaState_hpp__
