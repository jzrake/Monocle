#include "PlotModels.hpp"




//==============================================================================
var ModelHelpers::borderSizeToVar (const BorderSize<int>& border)
{
    return Array<var> {border.getTop(), border.getLeft(), border.getBottom(), border.getRight()};
}

BorderSize<int> ModelHelpers::borderSizeFromVar (const var& border)
{
    return {border[0], border[1], border[2], border[3]};
}

var ModelHelpers::rectangleToVar (const Rectangle<double>& rectangle)
{
    return Array<var> {rectangle.getX(), rectangle.getY(), rectangle.getWidth(), rectangle.getHeight()};
}

Rectangle<double> ModelHelpers::rectangleFromVar (const var& rectangle)
{
    return {rectangle[0], rectangle[1], rectangle[2], rectangle[3]};
}




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
