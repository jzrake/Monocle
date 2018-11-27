#include "AppSkeleton.hpp"
#include "MaterialIcons.hpp"
#define GAP_WIDTH 4




//==============================================================================
void DualComponentView::Resizer::paint (Graphics& g)
{
    g.setColour (Colours::lightgrey);
    g.drawHorizontalLine (getHeight() - 1, 0, getWidth());
}

MouseCursor DualComponentView::Resizer::getMouseCursor()
{
    return MouseCursor::UpDownResizeCursor;
}




//==============================================================================
DualComponentView::DualComponentView()
{
    resizer.addMouseListener (this, false);
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

void DualComponentView::resized()
{
    layout();
}

void DualComponentView::mouseDown (const MouseEvent& e)
{
    bottomHeightAtMouseDown = bottomHeight;
}

void DualComponentView::mouseDrag (const MouseEvent& e)
{
    bottomHeight = jlimit (100, getHeight() - 200, bottomHeightAtMouseDown - e.getDistanceFromDragStartY());
    layout();
    repaint();
}

bool DualComponentView::hitTest (int x, int y)
{
    return x < getWidth() - GAP_WIDTH;
}

void DualComponentView::resetContent()
{
    removeAllChildren();
    addAndMakeVisible (content1);
    addAndMakeVisible (content2);
    addAndMakeVisible (resizer);
    layout();
}

void DualComponentView::layout()
{
    auto area = getLocalBounds();

    if (content2)
        content2->setBounds (area.removeFromBottom (bottomHeight));
    if (content1)
        content1->setBounds (area);

    resizer.setBounds (area.removeFromBottom (GAP_WIDTH));
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
class AppSkeleton::BackdropButton : public Button, public Value::Listener
{
public:
    // ========================================================================
    BackdropButton() : Button ("Backdrop")
    {
        setTooltip ("Toggle backdrop");
        setClickingTogglesState (true);
        setWantsKeyboardFocus (false);
        getToggleStateValue().addListener (this);
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

    void valueChanged (Value&) override
    {
        findParentComponentOfClass<AppSkeleton>()->updatePageVisibility();
        findParentComponentOfClass<AppSkeleton>()->layout (true);
    }

private:
    // ========================================================================
    std::unique_ptr<Drawable> more;
    std::unique_ptr<Drawable> less;
    friend class AppSkeleton;
};




// ============================================================================
class AppSkeleton::TopNavComponent : public Component
{
public:
    void paint (Graphics& g) override
    {
        g.fillAll (Colours::white);
    }
};




// ============================================================================
AppSkeleton::AppSkeleton()
{
    alertLabel.setColour (Label::textColourId, Colours::darkred);
    alertLabel.setJustificationType (Justification::centred);
    topNavComponent = std::make_unique<TopNavComponent>();
    backdropButton  = std::make_unique<BackdropButton>();
    addAndMakeVisible (*topNavComponent);
    addAndMakeVisible (*backdropButton);
    addChildComponent (alertLabel);
}

AppSkeleton::~AppSkeleton()
{
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
            addChildComponent (&backdrop, 0);
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

void AppSkeleton::openNavSection (const String& name)
{
    if (name.isNotEmpty())
    {
        for (const auto& button : navButtons)
            if (button->getName() == name && ! button->getToggleState())
                button->triggerClick();
    }
}

void AppSkeleton::flashAlertLabel (const String& textToShow)
{
    alertLabel.setText (textToShow, NotificationType::sendNotification);
    alertLabel.setVisible (true);
    alertLabel.setAlpha (1.f);
    Timer::callAfterDelay (1500, [label=SafePointer<Component> (&alertLabel)] ()
    {
        if (label)
        {
            Desktop::getInstance().getAnimator().fadeOut (label, 500);
        }
    });
}

String AppSkeleton::getCurrentNavSectionName() const
{
    for (const auto& button : navButtons)
        if (button->getToggleState())
            return button->getName();

    return String();
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

// ============================================================================
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
        g.drawVerticalLine (geom.verticalGap.getRight(), geom.verticalGap.getY(), geom.verticalGap.getBottom());
}

void AppSkeleton::resized()
{
    layout();
}

void AppSkeleton::mouseEnter (const MouseEvent& e)
{
    mouseIsAtRightEdgeOfSourceList = isMouseAtRightEdgeOfSourceList (e.position);
}

void AppSkeleton::mouseExit (const MouseEvent& e)
{
    mouseIsAtRightEdgeOfSourceList = isMouseAtRightEdgeOfSourceList (e.position);
}

void AppSkeleton::mouseMove (const MouseEvent& e)
{
    mouseIsAtRightEdgeOfSourceList = isMouseAtRightEdgeOfSourceList (e.position);
}

void AppSkeleton::mouseDown (const MouseEvent& e)
{
    sourceListWidthAtMouseDown = sourceListWidth;
}

void AppSkeleton::mouseDrag (const MouseEvent& e)
{
    sourceListWidth = jlimit (100, getWidth() - 200, sourceListWidthAtMouseDown + e.getDistanceFromDragStartX());
    layout();
    repaint();
}

MouseCursor AppSkeleton::getMouseCursor()
{
    return mouseIsAtRightEdgeOfSourceList ? MouseCursor::LeftRightResizeCursor : MouseCursor::NormalCursor;
}

// ============================================================================
AppSkeleton::Geometry AppSkeleton::computeGeometry() const
{
    auto area = getLocalBounds();
    Geometry g;
    g.topNav      = area.removeFromTop (topNavHeight);
    g.leftNav     = area.removeFromLeft (leftNavWidth);
    g.sourceList  = area.removeFromLeft (sourceListVisible ? sourceListWidth : 0).withTrimmedLeft (1).withTrimmedTop (1);
    g.verticalGap = Rectangle<int> (g.sourceList).removeFromRight (GAP_WIDTH);
    g.mainContent = area.withTrimmedLeft (1).withTrimmedTop (1);
    g.backdrop    = g.mainContent.withHeight (backdropHeight).translated (0, -backdropHeight);
    g.backdropButton = Rectangle<int> (g.topNav).removeFromRight (topNavHeight);

    if (backdropButton->getToggleState())
    {
        g.backdrop.translate (0, backdropHeight);
        g.mainContent.translate (0, backdropHeight);
    }
    return g;
}

bool AppSkeleton::isMouseAtRightEdgeOfSourceList (const Point<float>& p) const
{
    auto geom = computeGeometry();
    return geom.verticalGap.contains (p.x, p.y);
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

void AppSkeleton::layout (bool animate)
{
    auto setBounds = [animate] (Component* c, const Rectangle<int>& b)
    {
//        if (animate)
//            Desktop::getInstance().getAnimator().animateComponent (c, b, 1.f, 150, false, 1, 0);
//        else
            c->setBounds (b);
    };

    auto geom = computeGeometry();

    for (const auto& button : navButtons)
        button->setBounds (geom.leftNav.removeFromTop (leftNavWidth));

    for (const auto& button : navButtons)
        if (auto page = button->page)
            page->setBounds (geom.sourceList);

    for (const auto& button : navButtons)
        if (auto backdrop = button->backdrop)
            setBounds (backdrop, geom.backdrop);

    if (mainContent)
        setBounds (mainContent, geom.mainContent);

    topNavComponent->setBounds (geom.topNav);
    backdropButton->setBounds (geom.backdropButton);
    alertLabel.setBounds (geom.topNav);
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
