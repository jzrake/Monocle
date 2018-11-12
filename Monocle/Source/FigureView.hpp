#pragma once
#include "JuceHeader.h"
#include "Main.hpp"
#include "PlotModels.hpp"




// ============================================================================
class Ticker
{
public:
    struct Tick
    {
        double value = 0.0;      /**< normalized data coordinate (0, 1) relative to limits */
        float pixel = 0.f;       /**< position in pixels on axes content */
        std::string label;
    };
    static std::vector<Tick> createTicks (double l0, double l1, int p0, int p1);
    static std::vector<Tick> formatTicks (const std::vector<double>& locations, double l0, double l1, int p0, int p1);
    static std::vector<double> locateTicksLog (double l0, double l1);
    static std::vector<double> locateTicks (double l0, double l1);
    static std::vector<float> getPixelLocations (const std::vector<Tick>& ticks);
};




//==============================================================================
class FigureView : public Component
{
public:

    //==========================================================================
    struct Geometry
    {
        Rectangle<int> marginT;
        Rectangle<int> marginB;
        Rectangle<int> marginL;
        Rectangle<int> marginR;
        Rectangle<int> ytickAreaL;
        Rectangle<int> ytickLabelAreaL;
        Rectangle<int> xtickAreaB;
        Rectangle<int> xtickLabelAreaB;
    };

    //==========================================================================
    class PlotArea : public Component
    {
    public:
        PlotArea (const FigureView&);
        void paint (Graphics&) override;
        void resized() override;
        void mouseDown (const MouseEvent&) override;
        void mouseDrag (const MouseEvent&) override;
        void mouseMagnify (const MouseEvent&, float) override;

    private:
        //======================================================================
        BorderSize<int> computeMargin() const;
        double toDomainX (double x) const;
        double toDomainY (double y) const;
        double fromDomainX (double x) const;
        double fromDomainY (double y) const;
        void dispatchSetMarginIfNeeded() const;
        void dispatchSetDomain (const Rectangle<double>& domain) const;

        //======================================================================
        const FigureView& figure;
        ComponentBoundsConstrainer constrainer;
        ResizableBorderComponent resizer;
        Rectangle<double> domainBeforePan;
    };

    //==========================================================================
    FigureView (const FigureModel& model);
    void setModel (const FigureModel&);
    void paint (Graphics&) override;
    void resized() override;

private:
    //==========================================================================
    void layout();
    Geometry computeGeometry() const;

    //==========================================================================
    FigureModel model;
    PlotArea plotArea;
    Label xlabel;
    Label ylabel;
    Label title;
};
