#include "MainComponent.hpp"




//==============================================================================
MainComponent::MainComponent()
{
    LinePlotModel linePlot;

    for (int n = 0; n < 200; ++n)
    {
        double t = 2 * M_PI * n / 200.0;
        linePlot.x.add (cos (t));
        linePlot.y.add (sin (t));
    }
    linePlot.lineWidth = 4;
    model.linePlots.add (linePlot);

    figure.reset (new FigureView (model));
    skeleton.setMainContent (figure.get());

    addAndMakeVisible (skeleton);

    setSize (800, 600);
}

MainComponent::~MainComponent()
{
}

void MainComponent::paint (Graphics& g)
{
}

void MainComponent::resized()
{
    skeleton.setBounds (getLocalBounds());
}

void MainComponent::dispatch (const Action& action)
{
    if (action.name == set_figure_margin)
    {
        model.margin = ModelHelpers::borderSizeFromVar (action.value);
        figure->setModel (model);
    }
    else if (action.name == set_figure_domain)
    {
        auto domain = ModelHelpers::rectangleFromVar (action.value);
        model.xmin = domain.getX();
        model.xmax = domain.getRight();
        model.ymin = domain.getY();
        model.ymax = domain.getBottom();
        figure->setModel (model);
    }
    else if (action.name == set_figure_xlabel)
    {
        model.xlabel = action.value;
        figure->setModel (model);
    }
    else if (action.name == set_figure_ylabel)
    {
        model.ylabel = action.value;
        figure->setModel (model);
    }
    else if (action.name == set_figure_title)
    {
        model.title = action.value;
        figure->setModel (model);
    }
}
