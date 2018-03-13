#include "LuaFrame.hpp"
#include "LuaState.hpp"

#include "utils/String.hpp"

namespace Wow
{
    namespace
    {
        const char * ToString( FrameAnchor anchor )
        {
            switch ( anchor )
            {
                case FrameAnchor::TOP: return "TOP";
                case FrameAnchor::CENTER: return "CENTER";
                case FrameAnchor::BOTTOM: return "BOTTOM";
                case FrameAnchor::TOPLEFT: return "TOPLEFT";
                case FrameAnchor::LEFT: return "LEFT";
                case FrameAnchor::BOTTOMLEFT: return "BOTTOMLEFT";
                case FrameAnchor::TOPRIGHT: return "TOPRIGHT";
                case FrameAnchor::RIGHT: return "RIGHT";
                case FrameAnchor::BOTTOMRIGHT: return "BOTTOMRIGHT";
            }

            return "";
        }
    }

    LuaChildFrame::LuaChildFrame( LuaRootFrame & parent, const std::string & childName )
        : LuaRootFrame( parent.m_lua )
        , m_parent( parent )
    {
        m_frameName = childName;
    }

    void LuaChildFrame::SetPosition( const Vector2i & pos, FrameAnchor anchor /*= FrameAnchor::TOPLEFT */ )
    {
        m_lua.Execute( Format( R"(%s:SetPoint( "%s", "%s", %i, %i ))", m_frameName.c_str(), ToString( anchor ), m_parent.GetName().c_str(), pos.x, pos.y ) );
    }

    LuaRootFrame::LuaRootFrame( LuaState & state, const std::string& frameName )
        : m_lua( state )
        , m_frameName( frameName )
    {
        m_lua.Execute( Format( R"(%s = CreateFrame("Frame", "%s", UIParent))", frameName.c_str(), frameName.c_str() ) );
        m_lua.Execute( Format( R"(%s:SetBackdrop(
        {
            bgFile = "Interface\\DialogFrame\\UI-DialogBox-Background",
            edgeFile = "Interface\\DialogFrame\\UI-DialogBox-Border",
            tile = true, tileSize = 32, edgeSize = 32,
            insets = { left = 11, right = 12, top = 12, bottom = 11 }
        } ) )", frameName.c_str() ) );
    }

    LuaRootFrame::LuaRootFrame( LuaState & state )
        : m_lua( state )
    {
    }

    void LuaRootFrame::SetSize( const Vector2i & size )
    {
        m_lua.Execute( Format( R"(%s:SetWidth( %i ))", m_frameName.c_str(), size.x ) );
        m_lua.Execute( Format( R"(%s:SetHeight( %i ))", m_frameName.c_str(), size.y ) );
    }

    void LuaRootFrame::SetPosition( const Vector2i & pos, FrameAnchor anchor )
    {
        m_lua.Execute( Format( R"(%s:SetPoint( "%s", %i, %i ))", m_frameName.c_str(), ToString(anchor), pos.x, pos.y ) );
    }

    void LuaRootFrame::SetText( const std::string & text )
    {
        m_lua.Execute( Format( R"(%s:SetText( "%s" ))", m_frameName.c_str(), text.c_str() ) );
    }

    LuaFrameLabel* LuaRootFrame::AddLabel( const std::string& frameName )
    {
        auto label = std::make_unique< LuaFrameLabel >( *this, frameName );

        LuaFrameLabel * result = label.get();
        m_frames.push_back( std::move( label ) );

        return result;
    }

    LuaCheckBox* LuaRootFrame::AddCheckBox( const std::string& frameName )
    {
        auto box = std::make_unique< LuaCheckBox >( *this, frameName );

        LuaCheckBox * result = box.get();
        m_frames.push_back( std::move( box ) );

        return result;
    }

    LuaFrameLabel::LuaFrameLabel( LuaRootFrame & parent, const std::string& labelName )
        : LuaChildFrame( parent, labelName )
    {
        auto & parentName = m_parent.GetName();
        m_lua.Execute( Format( R"(%s = %s:CreateFontString("%s", "BACKGROUND"))", m_frameName.c_str(), parentName.c_str(), m_frameName.c_str() ) );
        m_lua.Execute( Format( R"(%s:SetFontObject("GameFontNormal"))", m_frameName.c_str() ) );
    }

    LuaCheckBox::LuaCheckBox( LuaRootFrame & parent, const std::string& checkBoxName )
        : LuaChildFrame( parent, checkBoxName )
    {
        m_lua.Execute( Format( R"(%s = CreateFrame("CheckButton", "%s", %s, "ChatConfigCheckButtonTemplate"))", m_frameName.c_str(), m_frameName.c_str(), m_parent.GetName().c_str() ) );
    }

    bool LuaCheckBox::IsChecked()
    {
        auto results = m_lua.Execute( Format( R"( return %s:GetChecked() ~= nil )", m_frameName.c_str() ) );
        if ( results && !results->empty() )
        {
            return std::get< bool >( results->front() );
        }

        return false;
    }
}
