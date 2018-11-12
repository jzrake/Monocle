#pragma once
#include "JuceHeader.h"
#include "PlotModels.hpp"
#include "FigureView.hpp"




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
    std::unique_ptr<FigureView> figure;
    FigureModel model;
};
