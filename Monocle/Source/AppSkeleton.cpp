#include "AppSkeleton.hpp"
#include "MaterialIcons.hpp"




//==============================================================================
DualComponentView::DualComponentView()
{
}

void DualComponentView::setContent1 (Component& contentFor1)
{
    content1 = &contentFor1;
    resetContent();
}

void DualComponentView::setContent2 (Component& contentFor2)
{
    content2 = &contentFor2;
    resetContent();
}

void DualComponentView::paintOverChildren (Graphics& g)
{
    if (content2)
    {
        g.setColour (Colours::lightgrey);
        g.drawHorizontalLine (getHeight() - 300, 0, getWidth());
    }
}

void DualComponentView::resized()
{
    layout();
}

void DualComponentView::resetContent()
{
    removeAllChildren();
    addAndMakeVisible (content1);
    addAndMakeVisible (content2);
    layout();
}

void DualComponentView::layout()
{
    auto area = getLocalBounds();
    
    if (content2)
        content2->setBounds (area.removeFromBottom (300).withTrimmedTop (1));
    if (content1)
        content1->setBounds (area);
}




// ============================================================================
class AppSkeleton::NavButton : public Button
{
public:
    // ========================================================================
    NavButton (const String& tooltip, const String& svgString) : Button (tooltip)
    {
        setTooltip (tooltip);
        setWantsKeyboardFocus (false);
        drawable = material::util::icon (svgString, Colours::darkgrey);
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

        float w = 0.5f * drawable->getWidth();
        float h = 0.5f * drawable->getHeight();
        auto area = getLocalBounds().toFloat().withSizeKeepingCentre (w, h);
        drawable->drawWithin (g, area, RectanglePlacement::stretchToFit, 1.0f);
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

private:
    // ========================================================================
    std::unique_ptr<Drawable> drawable;
    WeakReference<Component> page;
    WeakReference<Component> backdrop;
    friend class AppSkeleton;
};




// ============================================================================
class AppSkeleton::BackdropButton : public Button
{
public:
    // ========================================================================
    BackdropButton() : Button ("Backdrop")
    {
        setTooltip ("Toggle backdrop");
        setClickingTogglesState (true);
        setWantsKeyboardFocus (false);
        more = material::util::icon (material::navigation::ic_expand_more, Colours::darkgrey);
        less = material::util::icon (material::navigation::ic_expand_less, Colours::darkgrey);
    }

    void paintButton (Graphics& g, bool highlighted, bool down) override
    {
        if (highlighted)
        {
            g.setColour (Colours::lightgrey);
            g.fillAll();
        }

        auto& d = getToggleState() ? less : more;
        float w = 0.5f * d->getWidth();
        float h = 0.5f * d->getHeight();
        auto area = getLocalBounds().toFloat().withSizeKeepingCentre (w, h);
        d->drawWithin (g, area, RectanglePlacement::stretchToFit, 1.0f);
    }

    void buttonStateChanged() override
    {
        findParentComponentOfClass<AppSkeleton>()->layout();
    }

private:
    // ========================================================================
    std::unique_ptr<Drawable> more;
    std::unique_ptr<Drawable> less;
    friend class AppSkeleton;
};




// ============================================================================
AppSkeleton::AppSkeleton()
{
    backdropButton = std::make_unique<BackdropButton>();
    addAndMakeVisible (*backdropButton);
}

AppSkeleton::~AppSkeleton()
{
}

void AppSkeleton::paint (Graphics& g)
{
}

void AppSkeleton::paintOverChildren (Graphics& g)
{
    auto geom = computeGeometry();

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
            button->page = &page;
            addChildComponent (&page);
            updatePageVisibility();
            return;
        }
    }
    jassertfalse; // There was no nav section with that name!
}

void AppSkeleton::setBackdrop (const String& name, Component& backdrop)
{
    for (auto& button : navButtons)
    {
        if (button->getName() == name)
        {
            button->backdrop = &backdrop;
            addChildComponent (&backdrop);
            updatePageVisibility();
            return;
        }
    }
    jassertfalse; // There was no nav section with that name!
}

void AppSkeleton::setBackdropRevealed (bool shouldBackdropBeRevealed)
{
    backdropButton->setToggleState (shouldBackdropBeRevealed, NotificationType::sendNotification);
}

void AppSkeleton::toggleNavPagesRevealed()
{
    for (auto& button : navButtons)
    {
        if (button->getToggleState())
        {
            lastNavButtonClosed = button.get();
            button->triggerClick();
            return;
        }
    }
    if (lastNavButtonClosed)
    {
        dynamic_cast<NavButton*>(lastNavButtonClosed.get())->triggerClick();
    }
    else if (! navButtons.isEmpty())
    {
        navButtons.getReference(0)->triggerClick();
    }
}

void AppSkeleton::toggleBackdropRevealed()
{
    backdropButton->triggerClick();
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
    g.backdropButton = Rectangle<int> (g.topNav).removeFromRight (topNavHeight);

    if (backdropButton->getToggleState())
    {
        g.backdrop = g.mainContent.withBottom (200);
        g.mainContent.translate (0, 200);
    }
    return g;
}

void AppSkeleton::showSourceList()
{
    sourceListVisible = true;
    layout();
    repaint();
}

void AppSkeleton::hideSourceList()
{
    sourceListVisible = false;
    layout();
    repaint();
}

void AppSkeleton::layout()
{
    auto geom = computeGeometry();

    for (const auto& button : navButtons)
        button->setBounds (geom.leftNav.removeFromTop (leftNavWidth));

    for (const auto& button : navButtons)
        if (auto page = button->page)
            page->setBounds (geom.sourceList);

    for (const auto& button : navButtons)
        if (auto backdrop = button->backdrop)
            backdrop->setBounds (geom.backdrop);

    if (mainContent)
        mainContent->setBounds (geom.mainContent);

    backdropButton->setBounds (geom.backdropButton);
}

void AppSkeleton::updatePageVisibility()
{
    for (const auto& button : navButtons)
        if (auto page = button->page)
            page->setVisible (button->getToggleState());

    for (const auto& button : navButtons)
        if (auto backdrop = button->backdrop)
            backdrop->setVisible (button->getToggleState());
}
