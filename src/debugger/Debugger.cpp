#include "Debugger.hpp"
#include <iostream>

namespace Debugger
{
    Debugger::Debugger( VTABLE_TYPE* vtable )
        : m_renderer( vtable )
        , m_memory( GetCurrentProcess() )
        , m_lua( m_memory )
    {
        m_lua.Execute( "AccountLoginAccountEdit:SetText('Root')" );
        m_lua.Execute( "AccountLoginPasswordEdit:SetText('Root')" );
        m_lua.Execute( "AccountLogin_Login()" );
    }
}
