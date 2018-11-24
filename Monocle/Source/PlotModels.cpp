#include "PlotModels.hpp"




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

    nd::ndarray<double, 1> x (200);
    nd::ndarray<double, 1> y (200);

    for (int n = 0; n < 200; ++n)
    {
        double t = 2 * M_PI * n / 200.0;
        x(n) = std::cos(t);
        y(n) = std::sin(t);
    }

    linePlot.lineWidth = 4;
    linePlot.x.become(x);
    linePlot.y.become(y);
    model.linePlots.add (linePlot);
    return model;
}
