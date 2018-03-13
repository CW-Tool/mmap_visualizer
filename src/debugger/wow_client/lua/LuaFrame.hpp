#ifndef LuaFrame_hpp__
#define LuaFrame_hpp__

#include "math/Vector.hpp"

#include <string>
#include <memory>
#include <vector>

namespace Wow
{
    class LuaState;

    enum class FrameAnchor
    {
        TOP,
        CENTER,
        BOTTOM,

        TOPLEFT,
        LEFT,
        BOTTOMLEFT,

        TOPRIGHT,
        RIGHT,
        BOTTOMRIGHT
    };

    class LuaFrameLabel;
    class LuaCheckBox;

    class LuaRootFrame
    {
        friend class LuaChildFrame;

    public:
        LuaRootFrame( LuaState & state, const std::string & frameName );

        virtual void        SetSize( const Vector2i & size );
        virtual void        SetPosition( const Vector2i & pos, FrameAnchor anchor = FrameAnchor::TOPLEFT );
        virtual void        SetText( const std::string & text );

        const std::string & GetName() { return m_frameName; }

        LuaFrameLabel*      AddLabel( const std::string& frameName );
        LuaCheckBox*        AddCheckBox( const std::string& frameName );

    protected:
        LuaRootFrame( LuaState & state );

        std::vector< std::unique_ptr< LuaChildFrame > > m_frames;

        std::string         m_frameName;
        LuaState &          m_lua;
    };

    class LuaChildFrame : public LuaRootFrame
    {
    public:
        LuaChildFrame( LuaRootFrame & parent, const std::string & childName );

        virtual void    SetPosition( const Vector2i & pos, FrameAnchor anchor = FrameAnchor::TOPLEFT ) override;

    protected:
        LuaRootFrame &  m_parent;
    };
    
    class LuaFrameLabel final : public LuaChildFrame
    {
    public:
        LuaFrameLabel( LuaRootFrame & parent, const std::string& labelName );
    };

    class LuaCheckBox final : public LuaChildFrame
    {
    public:
        LuaCheckBox( LuaRootFrame & parent, const std::string& checkBoxName );

        bool IsChecked();
    };
}

#endif // LuaFrame_hpp__