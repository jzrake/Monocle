#pragma once
#include "JuceHeader.h"




//==============================================================================
static Identifier set_figure_margin = "set_figure_margin";
static Identifier set_figure_domain = "set_figure_domain";




//==============================================================================
struct Action
{
    Action (const Identifier& name=Identifier()) : name (name) {}
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
