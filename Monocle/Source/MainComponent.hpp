#pragma once
#include "JuceHeader.h"
#include "PlotModels.hpp"
#include "FigureView.hpp"
#include "AppSkeleton.hpp"




//==============================================================================
class MainComponent : public Component, public StateManager
{
public:
    //==========================================================================
    MainComponent();
    ~MainComponent();

    void paint (Graphics&) override;
    void resized() override;

    //==========================================================================
    void dispatch (const Action& action) override;

private:
    //==========================================================================
    FigureView figure;
    TextEditor notesPage;

    FigureModel model;
    AppSkeleton skeleton;
};
