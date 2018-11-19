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




//==============================================================================
#include "NumericData.hpp"
#include "Kernel/Builtin.hpp"

using namespace mcl;

Object::Dict PlotModels::plot_models()
{
    Object::Dict m;
    m["line-plot"] = Object::Func (line_plot, "(x:double[n] y:double[n])");
    return m;
}

Object PlotModels::line_plot (const Object::List& args, const Object::Dict&)
{
    auto model = std::make_shared<LinePlotModel>();
    auto x = Builtin::check_user_data<ArrayDouble1> (args, 0);
    auto y = Builtin::check_user_data<ArrayDouble1> (args, 1);
    model->x.become (x.get());
    model->y.become (y.get());
    return Object::data (model);
}

Object PlotModels::figure (const Object::List& args, const Object::Dict&)
{
    return Object::data (std::make_shared<FigureModel>());
}
