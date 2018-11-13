#pragma once
#include "JuceHeader.h"




//==============================================================================
static Identifier set_figure_margin = "set_figure_margin";
static Identifier set_figure_domain = "set_figure_domain";
static Identifier set_figure_xlabel = "set_figure_xlabel";
static Identifier set_figure_ylabel = "set_figure_ylabel";
static Identifier set_figure_title  = "set_figure_title";




//==============================================================================
struct Action
{
    Action (const Identifier& name=Identifier(), var value=var()) : name (name), value (value) {}
    Identifier name;
    var value;
};




//==============================================================================
class StateManager
{
public:
    virtual ~StateManager() {}
    virtual void dispatch (const Action& action) = 0;
};




//==============================================================================
class ActionDispatcher
{
public:
    void dispatch (const Component* subject, const Action& action) const;
};




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
