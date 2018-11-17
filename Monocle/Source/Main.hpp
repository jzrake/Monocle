#pragma once
#include "JuceHeader.h"




//==============================================================================
namespace CommandIDs
{
    enum
    {
        openDocument              = 0x300000,
        windowToggleBackdrop      = 0x301001,
        windowToggleNavPages      = 0x301002,
        windowToggleOpenGL        = 0x301003,
    };
}




//==============================================================================
namespace CommandCategories
{
    static const char* const general       = "General";
    static const char* const editing       = "Editing";
    static const char* const view          = "View";
    static const char* const window        = "Window";
}




//==============================================================================
/**
 Not used yet...
*/
class MonocleLookAndFeel : public LookAndFeel_V4
{
public:
    void fillTextEditorBackground (Graphics& g, int width, int height, TextEditor& textEditor) override;
    void drawTextEditorOutline (Graphics& g, int width, int height, TextEditor& textEditor) override;
};
