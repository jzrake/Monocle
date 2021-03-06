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
        float pixel  = 0.f;      /**< position in pixels on axes content */
        std::string label;
    };
    static std::vector<Tick> createTicks (double l0, double l1, int p0, int p1);
    static std::vector<Tick> formatTicks (const std::vector<double>& locations, double l0, double l1, int p0, int p1);
    static std::vector<double> locateTicksLog (double l0, double l1);
    static std::vector<double> locateTicks (double l0, double l1);
    static std::vector<float> getPixelLocations (const std::vector<Tick>& ticks);
};




//==============================================================================
class FigureView : public Component, private Label::Listener
{
public:

    //==========================================================================
    class Listener
    {
    public:
        virtual ~Listener() {}
        virtual void figureViewSetMargin (FigureView* figure, const BorderSize<int>& value) = 0;
        virtual void figureViewSetDomain (FigureView* figure, const Rectangle<double>& value) = 0;
        virtual void figureViewSetXlabel (FigureView* figure, const String& value) = 0;
        virtual void figureViewSetYlabel (FigureView* figure, const String& value) = 0;
        virtual void figureViewSetTitle (FigureView* figure, const String& value) = 0;
    };

    //==========================================================================
    struct Geometry
    {
        Rectangle<int> marginT;
        Rectangle<int> marginB;
        Rectangle<int> marginL;
        Rectangle<int> marginR;
        Rectangle<int> ytickAreaL;
        Rectangle<int> xtickAreaB;
        Rectangle<int> ytickLabelAreaL;
        Rectangle<int> xtickLabelAreaB;
    };

    //==========================================================================
    class PlotArea : public Component
    {
    public:
        PlotArea (FigureView&);
        void paint (Graphics&) override;
        void resized() override;
        void mouseDown (const MouseEvent&) override;
        void mouseDrag (const MouseEvent&) override;
        void mouseMagnify (const MouseEvent&, float) override;

    private:
        //======================================================================
        void paintLinePlot (Graphics& g, const LinePlotModel& linePlot);
        void paintFillBetween (Graphics& g, const FillBetweenModel& fillBetween);
        void paintScatterPlot (Graphics& g, const ScatterPlotModel& scatterPlot);
        void paintHistogram (Graphics& g, const HistogramModel& histogram);
        void paintImagePlot (Graphics& g, const ImagePlotModel& imagePlot);

        //======================================================================
        BorderSize<int> computeMargin() const;
        double toDomainX (double x) const;
        double toDomainY (double y) const;
        double fromDomainX (double x) const;
        double fromDomainY (double y) const;
        void sendSetMarginIfNeeded();
        void sendSetDomain (const Rectangle<double>& domain);

        //======================================================================
        FigureView& figure;
        ComponentBoundsConstrainer constrainer;
        ResizableBorderComponent resizer;
        Rectangle<double> domainBeforePan;

        friend class FigureView;
    };

    //==========================================================================
    FigureView();
    void setModel (const FigureModel&);
    void addListener (Listener* listener);
    void removeListener (Listener* listener);

    //==========================================================================
    void paint (Graphics&) override;
    void paintOverChildren (Graphics&) override;
    void resized() override;
    void mouseEnter (const MouseEvent&) override;
    void mouseExit (const MouseEvent&) override;
    void mouseDown (const MouseEvent&) override;

private:
    //==========================================================================
    void layout();
    void refreshModes();
    Geometry computeGeometry() const;
    void labelTextChanged (Label* labelThatHasChanged) override;

    //==========================================================================
    FigureModel model;
    PlotArea plotArea;
    Label xlabel;
    Label ylabel;
    Label title;

    ListenerList<Listener> listeners;
    bool annotateGeometry = false;
    bool allowPlotAreaResize = true;
    bool paintAxisLabels = true;
    bool paintTickLabels = true;
    bool paintMarginsAndBackground = true;
};
