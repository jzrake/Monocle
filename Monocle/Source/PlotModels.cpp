#include "PlotModels.hpp"




// Experimental:
// =============================================================================
template <>
struct VariantConverter<FigureModel>
{
    static FigureModel fromVar (const var& v) { return FigureModel(); }
    static var toVar (const FigureModel& t)   { return var(); }
};




//==============================================================================
Rectangle<int> FigureModel::getTopMargin (const Rectangle<int>& area) const
{
    return {
        margin.getLeft(),
        0,
        area.getRight() - margin.getLeftAndRight(),
        margin.getTop()
    };
}

Rectangle<int> FigureModel::getBottomMargin (const Rectangle<int>& area) const
{
    return {
        margin.getLeft(),
        area.getBottom() - margin.getBottom(),
        area.getRight() - margin.getLeftAndRight(),
        margin.getBottom()
    };
}

Rectangle<int> FigureModel::getLeftMargin (const Rectangle<int>& area) const
{
    return {
        0,
        margin.getTop(),
        margin.getLeft(),
        area.getBottom() - margin.getTopAndBottom()
    };
}

Rectangle<int> FigureModel::getRightMargin (const Rectangle<int>& area) const
{
    return {
        area.getRight() - margin.getRight(),
        margin.getTop(),
        margin.getRight(),
        area.getBottom() - margin.getTopAndBottom()
    };
}

Rectangle<double> FigureModel::getDomain() const
{
    return Rectangle<double>(xmin, ymin, xmax - xmin, ymax - ymin);
}

FigureModel FigureModel::createExample()
{
    FigureModel model;
    LinePlotModel linePlot;

    for (int n = 0; n < 200; ++n)
    {
        double t = 2 * M_PI * n / 200.0;
        linePlot.x.add (cos (t));
        linePlot.y.add (sin (t));
    }
    linePlot.lineWidth = 4;
    model.linePlots.add (linePlot);
    return model;
}
