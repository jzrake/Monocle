#pragma once
#include "JuceHeader.h"
#include "Numerical/TabulatedFunction.hpp"
#include "3rdParty/ndarray/ndarray.hpp"
#include "Kernel/UserData.hpp"
#include "Kernel/Object.hpp"




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
struct LinePlotModel : public mcl::UserData
{
    nd::ndarray<double, 1> x;
    nd::ndarray<double, 1> y;
    float         lineWidth    = 1.f;
    float         markerSize   = 1.f;
    Colour        lineColour   = Colours::black;
    Colour        markerColour = Colours::black;
    LineStyle     lineStyle    = LineStyle::solid;
    MarkerStyle   markerStyle  = MarkerStyle::none;

    //==========================================================================
    std::string type() const override { return "LinePlotModel"; }
    std::string describe() const override { return "LinePlotModel"; }
    std::string serialize() const override { return ""; }
    bool load (const std::string&) override { return false; }
};




//==============================================================================
struct FillBetweenModel
{
    Array<double> x;
    Array<double> y0;
    Array<double> y1;
    float         lineWidth  = 1.f;
    Colour        fillColour = Colours::blue;
    Colour        lineColour = Colours::black;
    LineStyle     lineStyle  = LineStyle::none;
};




//==============================================================================
struct ScatterPlotModel
{
    Array<double> x;
    Array<double> y;
    Array<double> scalar;
    float         markerSize   = 1.f;
    Colour        markerColour = Colours::black;
    MarkerStyle   markerStyle  = MarkerStyle::square;
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
    unsigned int  ni   = 0;
    unsigned int  nj   = 0;
    double        vmin = 0.0;
    double        vmax = 1.0;
    ColourMap     colorMap;
};




//==============================================================================
struct FigureModel : public mcl::UserData
{
    Array<LinePlotModel>    linePlots;
    Array<FillBetweenModel> fillBetweens;
    Array<ScatterPlotModel> scatterPlots;
    Array<HistogramModel>   histograms;
    Array<ImagePlotModel>   imagePlots;
    double                  xmin             = 0.0;
    double                  xmax             = 1.0;
    double                  ymin             = 0.0;
    double                  ymax             = 1.0;
    String                  title            = "Figure";
    String                  xlabel           = "X Axis";
    String                  ylabel           = "Y Axis";
    BorderSize<int>         margin           = BorderSize<int> (40, 90, 50, 30);
    float                   borderWidth      = 1.f;
    float                   axesWidth        = 1.f;
    float                   gridlinesWidth   = 1.f;
    float                   tickLength       = 5.f;
    float                   tickWidth        = 1.f;
    float                   tickLabelPadding = 4.f;
    float                   tickLabelWidth   = 40.f;
    float                   tickLabelHeight  = 20.f;
    Colour                  marginColour     = Colours::whitesmoke;
    Colour                  borderColour     = Colours::black;
    Colour                  backgroundColour = Colours::white;
    Colour                  gridlinesColour  = Colours::lightgrey;

    //==========================================================================
    Rectangle<int> getTopMargin (const Rectangle<int>& area) const;
    Rectangle<int> getBottomMargin (const Rectangle<int>& area) const;
    Rectangle<int> getLeftMargin (const Rectangle<int>& area) const;
    Rectangle<int> getRightMargin (const Rectangle<int>& area) const;
    Rectangle<double> getDomain() const;

    //==========================================================================
    std::string type() const override { return "FigureModel"; }
    std::string describe() const override { return "FigureModel"; }
    std::string serialize() const override { return ""; }
    bool load (const std::string&) override { return false; }

    static FigureModel createExample();
};




//==========================================================================
class PlotModels
{
public:
    using Object = mcl::Object;

    static Object::Dict plot_models();
    static Object line_plot (const Object::List&, const Object::Dict&);
    static Object figure (const Object::List&, const Object::Dict&);
};
