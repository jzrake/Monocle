#include "AppSkeleton.hpp"
#include "MaterialIcons.hpp"




// ============================================================================
class AppSkeleton::NavButton : public Button
{
public:
    NavButton (const String& tooltip, const String& svgString) : Button (tooltip)
    {
        setTooltip (tooltip);
        auto svg = std::unique_ptr<XmlElement> (XmlDocument (svgString).getDocumentElement());
        drawable = std::unique_ptr<Drawable> (Drawable::createFromSVG (*svg));
        drawable->replaceColour (Colours::black, Colours::darkgrey);
    }

    void paintButton (Graphics& g,
                      bool shouldDrawButtonAsHighlighted,
                      bool shouldDrawButtonAsDown) override
    {
        if (shouldDrawButtonAsHighlighted)
            g.fillAll (Colours::lightgrey);

        drawable->drawWithin (g, getLocalBounds().toFloat(), RectanglePlacement::doNotResize, 1.0f);
    }

    void resized() override
    {
        auto a = 0.6f;
        drawable->setTransform (AffineTransform::scale (a, a, 0.5f * drawable->getWidth(), 0.5f * drawable->getHeight()));
    }
private:
    std::unique_ptr<Drawable> drawable;
};




// ============================================================================
AppSkeleton::AppSkeleton()
{
    navButtons.push_back (std::make_unique<NavButton> ("Sources", material::bintos (material::action::ic_list)));
    navButtons.push_back (std::make_unique<NavButton> ("Files", material::bintos (material::file::ic_folder_open)));
    navButtons.push_back (std::make_unique<NavButton> ("Settings", material::bintos (material::action::ic_settings)));

    for (const auto& button : navButtons)
        addAndMakeVisible (*button);
}

void AppSkeleton::paint (Graphics& g)
{
    auto geom = computeGeometry();

    g.fillAll (Colours::white);
    g.setColour (Colours::lightgrey);
    g.drawHorizontalLine (geom.topNav.getBottom(), geom.topNav.getX(), geom.topNav.getRight());
    g.drawVerticalLine (geom.leftNav.getRight(), geom.leftNav.getY(), geom.leftNav.getBottom());
    g.drawVerticalLine (geom.sourceList.getRight(), geom.sourceList.getY(), geom.sourceList.getBottom());
}

void AppSkeleton::resized()
{
    auto geom = computeGeometry();

    for (const auto& button : navButtons)
        button->setBounds (geom.leftNav.removeFromTop (leftNavWidth));

    if (mainContent)
        mainContent->setBounds (geom.mainContent);
}

void AppSkeleton::setMainContent (Component* mainContentToShow)
{
    addAndMakeVisible (mainContent = mainContentToShow);
}

AppSkeleton::Geometry AppSkeleton::computeGeometry() const
{
    auto area = getLocalBounds();
    Geometry g;
    g.topNav      = area.removeFromTop (topNavHeight);
    g.leftNav     = area.removeFromLeft (leftNavWidth);
    g.sourceList  = area.removeFromLeft (sourceListWidth);
    g.mainContent = area.withTrimmedLeft (1).withTrimmedTop (1);
    return g;
}
