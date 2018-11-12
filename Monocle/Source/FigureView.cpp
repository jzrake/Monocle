#include "FigureView.hpp"




//==========================================================================
static std::vector<Rectangle<float>> makeRectanglesInColumn (const Rectangle<int>& column,
                                                             const std::vector<float>& midpoints,
                                                             float height)
{
    std::vector<Rectangle<float>> rectangles;
    
    for (auto y : midpoints)
        rectangles.push_back (Rectangle<float> (column.getX(), y - height * 0.5f, column.getWidth(), height));
    return rectangles;
}

static std::vector<Rectangle<float>> makeRectanglesInRow (const Rectangle<int>& row,
                                                          const std::vector<float>& midpoints,
                                                          float width)
{
    std::vector<Rectangle<float>> rectangles;
    
    for (auto x : midpoints)
        rectangles.push_back (Rectangle<float> (x - width * 0.5f, row.getY(), width, row.getHeight()));
    return rectangles;
}




//==========================================================================
std::vector<Ticker::Tick> Ticker::createTicks (double l0, double l1, int p0, int p1)
{
    return formatTicks (locateTicks (l0, l1), l0, l1, p0, p1);
}

std::vector<Ticker::Tick> Ticker::formatTicks (const std::vector<double>& locations,
                                               double l0, double l1, int p0, int p1)
{
    auto ticks = std::vector<Tick>();
    char buffer[256];

    for (auto x : locations)
    {
        std::snprintf (buffer, 256, "%.8lf", x);

        Tick t;
        t.value = (x - l0) / (l1 - l0);
        t.pixel = p0 + t.value * (p1 - p0);
        t.label = buffer;

        while (t.label.back() == '0')
            t.label.pop_back();

        if (t.label.back() == '.')
            t.label.push_back ('0');

        ticks.push_back (t);
    }
    return ticks;
}

std::vector<double> Ticker::locateTicksLog (double l0, double l1)
{
    auto N0 = 8;
    auto x0 = std::floor (std::min (l0, l1));
    auto x1 = std::ceil  (std::max (l0, l1));
    auto decskip = 1 + (x1 - x0) / N0;

    auto loc = std::vector<double>();

    for (int n = x0; n < x1; n += decskip)
        loc.push_back (n);

    return loc;
}

std::vector<double> Ticker::locateTicks (double l0, double l1)
{
    auto N0 = 10;
    auto x0 = std::min (l0, l1);
    auto x1 = std::max (l0, l1);
    auto dx = std::pow (10, -1 + std::floor (std::log10 (x1 - x0) + 1e-8));
    auto Nx = (x1 - x0) / dx;

    while (Nx <= N0) { Nx *= 2; dx /= 2; }
    while (Nx >  N0) { Nx /= 2; dx *= 2; }

    auto start = int (x0 / dx) * dx;
    auto loc = std::vector<double>();

    for (int n = 0; n <= Nx + 1; ++n)
    {
        auto x = start + n * dx;

        if (x0 + 0.1 * dx <= x && x <= x1 - 0.1 * dx)
            loc.push_back (x);
    }
    return loc;
}

std::vector<float> Ticker::getPixelLocations (const std::vector<Tick>& ticks)
{
    std::vector<float> pixels;
    std::transform (ticks.begin(), ticks.end(), std::back_inserter (pixels), [] (const auto& t) { return t.pixel; });
    return pixels;
}




//==========================================================================
FigureView::PlotArea::PlotArea (const FigureView& figure)
: figure (figure)
, resizer (this, &constrainer)
{
    constrainer.setMinimumOnscreenAmounts (0xffffff, 0xffffff, 0xffffff, 0xffffff);
    constrainer.setMinimumSize (100, 100);
    addChildComponent (resizer);
}

void FigureView::PlotArea::paint (Graphics& g)
{
    g.setColour (figure.model.backgroundColour);
    g.fillRect (getLocalBounds());


    auto xticks = Ticker::createTicks (figure.model.xmin, figure.model.xmax, 0, getWidth());
    auto yticks = Ticker::createTicks (figure.model.ymin, figure.model.ymax, getHeight(), 0);

    
    // Draw gridlines
    // =================================================================
    g.setColour (figure.model.gridlinesColour);
    for (const auto& tick : xticks) g.drawVerticalLine (tick.pixel, 0, getHeight());
    for (const auto& tick : yticks) g.drawHorizontalLine (tick.pixel, 0, getWidth());

    for (const auto& p : figure.model.linePlots) paintLinePlot (g, p);
    for (const auto& p : figure.model.fillBetweens) paintFillBetween (g, p);
    for (const auto& p : figure.model.scatterPlots) paintScatterPlot (g, p);
    for (const auto& p : figure.model.histograms) paintHistogram (g, p);
    for (const auto& p : figure.model.imagePlots) paintImagePlot (g, p);
}

void FigureView::PlotArea::resized()
{
    resizer.setBounds (getLocalBounds());
    dispatchSetMarginIfNeeded();
}

void FigureView::PlotArea::mouseDown (const MouseEvent&)
{
    domainBeforePan = figure.model.getDomain();
}

void FigureView::PlotArea::mouseDrag (const MouseEvent& e)
{
    const auto D = Point<double> (getWidth(), getHeight());
    const auto m = Point<double> (e.getDistanceFromDragStartX(), -e.getDistanceFromDragStartY());
    const auto p = domainBeforePan.getTopLeft();
    const auto q = domainBeforePan.getBottomRight();
    const auto d = q - p;
    dispatchSetDomain (figure.model.getDomain().withPosition (p - d * m / D));
}

void FigureView::PlotArea::mouseMagnify (const MouseEvent& e, float scaleFactor)
{
    auto domain = figure.model.getDomain();
    const double xlim[2] = { domain.getX(), domain.getRight() };
    const double ylim[2] = { domain.getY(), domain.getBottom() };
    const double Dx = getWidth();
    const double Dy = getHeight();
    const double dx = domain.getWidth();
    const double dy = domain.getHeight();
    const double newdx = dx / scaleFactor;
    const double newdy = dy / scaleFactor;
    const double fixedx = toDomainX (e.position.x);
    const double fixedy = toDomainY (e.position.y);
    const double newx0 = e.mods.isAltDown()  ? xlim[0] : fixedx - newdx * (0 + e.position.x / Dx);
    const double newx1 = e.mods.isAltDown()  ? xlim[1] : fixedx + newdx * (1 - e.position.x / Dx);
    const double newy0 = e.mods.isCtrlDown() ? ylim[0] : fixedy - newdy * (1 - e.position.y / Dy);
    const double newy1 = e.mods.isCtrlDown() ? ylim[1] : fixedy + newdy * (0 + e.position.y / Dy);
    dispatchSetDomain (Rectangle<double>::leftTopRightBottom (newx0, newy0, newx1, newy1));
}




//==========================================================================
void FigureView::PlotArea::paintLinePlot (Graphics& g, const LinePlotModel& linePlot)
{
    jassert (linePlot.x.size() == linePlot.y.size());

    Path p;
    p.startNewSubPath (fromDomainX (linePlot.x.getFirst()),
                       fromDomainY (linePlot.y.getFirst()));

    for (int n = 1; n < linePlot.x.size(); ++n)
    {
        p.lineTo (fromDomainX (linePlot.x.getUnchecked (n)),
                  fromDomainY (linePlot.y.getUnchecked (n)));
    }
    g.setColour (linePlot.lineColour);
    g.strokePath (p, PathStrokeType (linePlot.lineWidth));
}
void FigureView::PlotArea::paintFillBetween (Graphics& g, const FillBetweenModel& fillBetween) {}
void FigureView::PlotArea::paintScatterPlot (Graphics& g, const ScatterPlotModel& scatterPlot) {}
void FigureView::PlotArea::paintHistogram (Graphics& g, const HistogramModel& histogram) {}
void FigureView::PlotArea::paintImagePlot (Graphics& g, const ImagePlotModel& imagePlot) {}




//==========================================================================
BorderSize<int> FigureView::PlotArea::computeMargin() const
{
    return {getY(), getX(), getParentHeight() - getBottom(), getParentWidth() - getRight()};
}

double FigureView::PlotArea::fromDomainX (double x) const
{
    return jmap (x, figure.model.xmin, figure.model.xmax, 0.0, double (getWidth()));
}

double FigureView::PlotArea::fromDomainY (double y) const
{
    return jmap (y, figure.model.ymin, figure.model.ymax, double (getHeight()), 0.0);
}

double FigureView::PlotArea::toDomainX (double x) const
{
    return jmap (x, 0.0, double (getWidth()), figure.model.xmin, figure.model.xmax);
}

double FigureView::PlotArea::toDomainY (double y) const
{
    return jmap (y, double (getHeight()), 0.0, figure.model.ymin, figure.model.ymax);
}

void FigureView::PlotArea::dispatchSetMarginIfNeeded() const
{
    auto newMargin = computeMargin();

    if (newMargin != figure.model.margin)
        dispatch (this, {set_figure_margin, ModelHelpers::borderSizeToVar (newMargin)});
}

void FigureView::PlotArea::dispatchSetDomain (const Rectangle<double>& domain) const
{
    dispatch (this, {set_figure_domain, ModelHelpers::rectangleToVar (domain)});
}




//==========================================================================
FigureView::FigureView (const FigureModel& model) : model (model), plotArea (*this)
{
    xlabel.setJustificationType (Justification::centred);
    ylabel.setJustificationType (Justification::centred);
    title .setJustificationType (Justification::centred);
    xlabel.setColour (Label::ColourIds::textColourId, Colours::black);
    ylabel.setColour (Label::ColourIds::textColourId, Colours::black);
    title .setColour (Label::ColourIds::textColourId, Colours::black);
    xlabel.setColour (Label::ColourIds::textWhenEditingColourId, Colours::black);
    ylabel.setColour (Label::ColourIds::textWhenEditingColourId, Colours::black);
    title .setColour (Label::ColourIds::textWhenEditingColourId, Colours::black);

    xlabel.setPaintingIsUnclipped (true);
    ylabel.setPaintingIsUnclipped (true);
    title .setPaintingIsUnclipped (true);
    xlabel.setEditable (true);
    ylabel.setEditable (true);
    title .setEditable (true);
    xlabel.addListener (this);
    ylabel.addListener (this);
    title .addListener (this);

    // So that we get popup menu clicks
    plotArea.addMouseListener (this, false);

    setModel (model);
    refreshModes();

    addAndMakeVisible (plotArea);
    addAndMakeVisible (title);
    addAndMakeVisible (xlabel);
    addAndMakeVisible (ylabel);
}

void FigureView::setModel (const FigureModel& newModel)
{
    model = newModel;
    xlabel.setText (model.xlabel, NotificationType::dontSendNotification);
    ylabel.setText (model.ylabel, NotificationType::dontSendNotification);
    title .setText (model.title , NotificationType::dontSendNotification);
    layout();
    repaint();
}

void FigureView::paint (Graphics& g)
{
    g.setColour (model.marginColour);
    g.fillAll();
}

void FigureView::paintOverChildren (Graphics& g)
{
    auto geom = computeGeometry();
    
    
    // Compute tick geometry data
    // =================================================================
    auto xticks          = Ticker::createTicks (model.xmin, model.xmax, plotArea.getX(), plotArea.getRight());
    auto xtickPixels     = Ticker::getPixelLocations (xticks);
    auto xtickLabelBoxes = makeRectanglesInRow (geom.xtickLabelAreaB, xtickPixels, model.tickLabelWidth);
    auto xtickBoxes      = makeRectanglesInRow (geom.xtickAreaB, xtickPixels, model.tickWidth);

    auto yticks          = Ticker::createTicks (model.ymin, model.ymax, plotArea.getBottom(), plotArea.getY());
    auto ytickPixels     = Ticker::getPixelLocations (yticks);
    auto ytickLabelBoxes = makeRectanglesInColumn (geom.ytickLabelAreaL, ytickPixels, model.tickLabelHeight);
    auto ytickBoxes      = makeRectanglesInColumn (geom.ytickAreaL, ytickPixels, model.tickWidth);
    
    
    // Extra geometry fills for debugging geometry
    // =================================================================
    if (annotateGeometry)
    {
        g.setColour (Colours::blue.withAlpha (0.3f));
        g.fillRect (geom.xtickAreaB);
        g.fillRect (geom.ytickAreaL);
        
        g.setColour (Colours::red.withAlpha (0.3f));
        g.fillRect (geom.xtickLabelAreaB);
        g.fillRect (geom.ytickLabelAreaL);
        
        g.setColour (Colours::yellow.withAlpha (0.3f));
        g.fillRect (geom.marginT);
        g.fillRect (geom.marginB);
        g.fillRect (geom.marginL);
        g.fillRect (geom.marginR);
        
        g.setColour (Colours::purple.withAlpha (0.3f));
        for (auto box : ytickLabelBoxes) g.fillRect (box);
        for (auto box : xtickLabelBoxes) g.fillRect (box);
    }
    
    
    // Draw the ticks and labels
    // =================================================================
    g.setColour (Colours::black);
    for (auto box : xtickBoxes) g.fillRect (box);
    for (auto box : ytickBoxes) g.fillRect (box);
    for (int n = 0; n < xticks.size(); ++n) g.drawText (xticks[n].label, xtickLabelBoxes[n], Justification::centredTop);
    for (int n = 0; n < yticks.size(); ++n) g.drawText (yticks[n].label, ytickLabelBoxes[n], Justification::centredRight);
}

void FigureView::resized()
{
    layout();
}

void FigureView::mouseDown (const MouseEvent& e)
{
    if (e.mods.isPopupMenu())
    {
        PopupMenu menu;
        menu.addItem (1, "Annotate geometry", true, annotateGeometry);
        menu.addItem (2, "Plot area resizer", true, allowPlotAreaResize);

        menu.showMenuAsync (PopupMenu::Options(), [this] (int code)
        {
            switch (code)
            {
                case 1: annotateGeometry = ! annotateGeometry; repaint(); break;
                case 2: allowPlotAreaResize = ! allowPlotAreaResize; refreshModes(); break;
                default: break;
            }
        });
    }
}




//==============================================================================
void FigureView::layout()
{
    auto g = computeGeometry();

    AffineTransform ylabelRot = AffineTransform::rotation (-M_PI_2, g.marginL.getCentreX(), g.marginL.getCentreY());
    ylabel.setTransform (ylabelRot);

    plotArea.setBounds (model.margin.subtractedFrom (getLocalBounds()));
    xlabel.setBounds (g.marginB);
    ylabel.setBounds (g.marginL.transformedBy (ylabelRot.inverted()));
    title.setBounds (g.marginT);
}

void FigureView::refreshModes()
{
    plotArea.resizer.setVisible (allowPlotAreaResize);
}

FigureView::Geometry FigureView::computeGeometry() const
{
    auto area = getLocalBounds();
    Geometry g;
    g.marginT         = model.getTopMargin (area);
    g.marginB         = model.getBottomMargin (area);
    g.marginL         = model.getLeftMargin (area);
    g.marginR         = model.getRightMargin (area);
    g.ytickAreaL      = g.marginL.removeFromRight (model.tickLength);
    g.ytickLabelAreaL = g.marginL.removeFromRight (model.tickLabelWidth).withTrimmedRight (model.tickLabelPadding);
    g.xtickAreaB      = g.marginB.removeFromTop (model.tickLength);
    g.xtickLabelAreaB = g.marginB.removeFromTop (model.tickLabelHeight).withTrimmedTop (model.tickLabelPadding);
    return g;
}

void FigureView::labelTextChanged (Label* label)
{
    if (label == &xlabel) dispatch (this, {set_figure_xlabel, xlabel.getText()});
    if (label == &ylabel) dispatch (this, {set_figure_ylabel, xlabel.getText()});
    if (label == &title ) dispatch (this, {set_figure_title , title .getText()});
}
