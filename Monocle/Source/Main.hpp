#pragma once
#include "JuceHeader.h"




//==============================================================================
namespace CommandIDs
{
    enum
    {
        openDocument              = 0x301000,

        windowToggleBackdrop      = 0x302001,
        windowToggleNavPages      = 0x302002,
        windowToggleOpenGL        = 0x302003,

        kernelCreateRule          = 0x303000,
    };
}




//==============================================================================
namespace CommandCategories
{
    static const char* const general       = "General";
    static const char* const edit          = "Edit";
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
