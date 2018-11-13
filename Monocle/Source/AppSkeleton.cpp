#include "AppSkeleton.hpp"




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

    void paintButton (Graphics& g, bool highlighted, bool down) override
    {
        if (highlighted)
        {
            g.setColour (Colours::lightgrey);
            g.fillAll();
        }
        if (getToggleState())
        {
            g.setColour (Colours::green);
            g.fillRect (getLocalBounds().removeFromLeft (4));
        }
        drawable->drawWithin (g, getLocalBounds().toFloat(), RectanglePlacement::doNotResize, 1.0f);
    }

    void clicked() override
    {
        auto skeleton = findParentComponentOfClass<AppSkeleton>();

        if (getToggleState())
        {
            setToggleState (false, NotificationType::dontSendNotification);
            skeleton->hideSourceList();
        }
        else
        {
            setToggleState (true, NotificationType::dontSendNotification);

            for (auto& button : skeleton->navButtons)
                if (button.get() != this)
                    button->setToggleState (false, NotificationType::dontSendNotification);

            skeleton->showSourceList();
            skeleton->updatePageVisibility();
        }
    }

    void resized() override
    {
        auto a = 0.6f;
        auto w = drawable->getWidth();
        auto h = drawable->getHeight();
        drawable->setTransform (AffineTransform::scale (a, a, 0.5f * w, 0.5f * h));
    }
private:
    std::unique_ptr<Drawable> drawable;
    WeakReference<Component> page;
    friend class AppSkeleton;
};




// ============================================================================
AppSkeleton::AppSkeleton()
{
}

void AppSkeleton::paint (Graphics& g)
{
    auto geom = computeGeometry();

    g.fillAll (Colours::white);
    g.setColour (Colours::lightgrey);
    g.drawHorizontalLine (geom.topNav.getBottom(), geom.topNav.getX(), geom.topNav.getRight());
    g.drawVerticalLine (geom.leftNav.getRight(), geom.leftNav.getY(), geom.leftNav.getBottom());

    if (sourceListVisible)
        g.drawVerticalLine (geom.sourceList.getRight(), geom.sourceList.getY(), geom.sourceList.getBottom());
}

void AppSkeleton::resized()
{
    layout();
}

void AppSkeleton::setMainContent (Component& mainContentToShow)
{
    addAndMakeVisible (mainContent = &mainContentToShow);
}

void AppSkeleton::setNavPage (const String& name, Component& page)
{
    for (auto& button : navButtons)
    {
        if (button->getName() == name)
        {
            addChildComponent (&page);
            dynamic_cast<NavButton*>(button.get())->page = &page;
            updatePageVisibility();
            return;
        }
    }
    jassertfalse; // There was no page with that name!
}

void AppSkeleton::addNavButton (const String& name, const String& svg)
{
    auto button = std::make_unique<NavButton> (name, svg);
    button->setTriggeredOnMouseDown (true);
    button->setTooltip (name);
    addAndMakeVisible (*button);
    navButtons.add (std::move (button));
}

AppSkeleton::Geometry AppSkeleton::computeGeometry() const
{
    auto area = getLocalBounds();
    Geometry g;
    g.topNav      = area.removeFromTop (topNavHeight);
    g.leftNav     = area.removeFromLeft (leftNavWidth);
    g.sourceList  = area.removeFromLeft (sourceListVisible ? sourceListWidth : 0).withTrimmedLeft (1).withTrimmedTop (1);;
    g.mainContent = area.withTrimmedLeft (1).withTrimmedTop (1);
    return g;
}

void AppSkeleton::showSourceList()
{
    sourceListVisible = true;
    resized();
}

void AppSkeleton::hideSourceList()
{
    sourceListVisible = false;
    resized();
}

void AppSkeleton::layout()
{
    auto geom = computeGeometry();

    for (const auto& button : navButtons)
        button->setBounds (geom.leftNav.removeFromTop (leftNavWidth));

    for (const auto& button : navButtons)
        if (auto page = dynamic_cast<NavButton*>(button.get())->page)
            page->setBounds (geom.sourceList);

    if (mainContent)
        mainContent->setBounds (geom.mainContent);
}

void AppSkeleton::updatePageVisibility()
{
    for (const auto& button : navButtons)
        if (auto page = dynamic_cast<NavButton*>(button.get())->page)
            page->setVisible (button->getToggleState());
}
