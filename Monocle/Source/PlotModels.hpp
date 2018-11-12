#pragma once
#include "JuceHeader.h"
#include "Numerical/TabulatedFunction.hpp"




//==============================================================================
class ModelHelpers
{
public:
    static var borderSizeToVar (const BorderSize<int>& border)
    {
        return Array<var> {border.getTop(), border.getLeft(), border.getBottom(), border.getRight()};
    }

    static BorderSize<int> borderSizeFromVar (const var& border)
    {
        return {border[0], border[1], border[2], border[3]};
    }

    static var rectangleToVar (const Rectangle<double>& rectangle)
    {
        return Array<var> {rectangle.getX(), rectangle.getY(), rectangle.getWidth(), rectangle.getHeight()};
    }

    static Rectangle<double> rectangleFromVar (const var& rectangle)
    {
        return {rectangle[0], rectangle[1], rectangle[2], rectangle[3]};
    }
};




//==============================================================================
enum class LineStyle { none, solid, dash, dashdot };
enum class MarkerStyle { none, circle, square, diamond, plus, cross };




//==============================================================================
struct ColourMap
{
    TabulatedFunction r;
    TabulatedFunction g;
    TabulatedFunction b;
};




//==============================================================================
struct LinePlotModel
{
    Array<double> x;
    Array<double> y;
    float         lineWidth = 1.f;
    float         markerSize = 1.f;
    Colour        lineColour = Colours::black;
    Colour        markerColour = Colours::black;
    LineStyle     lineStyle = LineStyle::solid;
    MarkerStyle   markerStyle = MarkerStyle::none;
};




//==============================================================================
struct FillBetweenModel
{
    Array<double> x;
    Array<double> y0;
    Array<double> y1;
    float         lineWidth = 1.f;
    Colour        fillColour;
    Colour        lineColour;
    LineStyle     lineStyle;
};




//==============================================================================
struct ScatterPlotModel
{
    Array<double> x;
    Array<double> y;
    Array<double> scalar;
    float         markerSize = 1.f;
    Colour        markerColour;
    MarkerStyle   markerStyle;
    ColourMap     colorMap;
};




//==============================================================================
struct HistogramModel
{
    Array<double> binEdges;
    Array<double> binMasses;
    float         lineWidth;
    Colour        lineColour;
    Colour        fillColour;
};




//==============================================================================
struct ImagePlotModel
{
    Array<double> scalar;
    unsigned int  ni = 0;
    unsigned int  nj = 0;
    double        vmin = 0.0;
    double        vmax = 1.0;
    ColourMap     colorMap;
};




//==============================================================================
struct FigureModel
{
    Array<LinePlotModel>    linePlots;
    Array<FillBetweenModel> fillBetweens;
    Array<ScatterPlotModel> scatterPlots;
    Array<HistogramModel>   histograms;
    Array<ImagePlotModel>   imagePlots;
    double                  xmin = 0.0;
    double                  xmax = 1.0;
    double                  ymin = 0.0;
    double                  ymax = 1.0;
    String                  title = "Figure";
    String                  xlabel = "X Axis";
    String                  ylabel = "Y Axis";
    Array<double>           xticks;
    Array<double>           yticks;
    BorderSize<int>         margin = BorderSize<int>(40, 90, 50, 30);
    float                   borderWidth      = 1.f;
    float                   axesWidth        = 1.f;
    float                   tickLength       = 5.f;
    float                   tickWidth        = 2.f;
    float                   tickLabelPadding = 4.f;
    float                   tickLabelWidth   = 40.f;
    float                   tickLabelHeight  = 20.f;
    Colour                  marginColour     = Colours::tan;
    Colour                  borderColour     = Colours::black;
    Colour                  backgroundColour = Colours::lightblue;


    Rectangle<int> getTopMargin (const Rectangle<int>& area) const
    {
        return {
            margin.getLeft(),
            0,
            area.getRight() - margin.getLeftAndRight(),
            margin.getTop()
        };
    }

    Rectangle<int> getBottomMargin (const Rectangle<int>& area) const
    {
        return {
            margin.getLeft(),
            area.getBottom() - margin.getBottom(),
            area.getRight() - margin.getLeftAndRight(),
            margin.getBottom()
        };
    }

    Rectangle<int> getLeftMargin (const Rectangle<int>& area) const
    {
        return {
            0,
            margin.getTop(),
            margin.getLeft(),
            area.getBottom() - margin.getTopAndBottom()
        };
    }

    Rectangle<int> getRightMargin (const Rectangle<int>& area) const
    {
        return {
            area.getRight() - margin.getRight(),
            margin.getTop(),
            margin.getRight(),
            area.getBottom() - margin.getTopAndBottom()
        };
    }

    Rectangle<double> getDomain() const
    {
        return Rectangle<double>(xmin, ymin, xmax - xmin, ymax - ymin);
    }
};
