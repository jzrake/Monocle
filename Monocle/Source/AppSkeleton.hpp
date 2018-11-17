#pragma once
#include "JuceHeader.h"




//==============================================================================
class DualComponentView : public Component
{
public:
    DualComponentView();
    void setContent1 (Component& contentFor1);
    void setContent2 (Component& contentFor2);
    void paintOverChildren (Graphics&) override;
    void resized() override;
private:
    void resetContent();
    void layout();
    WeakReference<Component> content1;
    WeakReference<Component> content2;
};




// ============================================================================
class AppSkeleton : public Component
{
public:
    class NavButton;
    class BackdropButton;

    // ========================================================================
    struct Geometry
    {
        Rectangle<int> topNav;
        Rectangle<int> leftNav;
        Rectangle<int> sourceList;
        Rectangle<int> mainContent;
        Rectangle<int> backdrop;
        Rectangle<int> backdropButton;
    };

    // ========================================================================
    AppSkeleton();
    ~AppSkeleton();
    void paint (Graphics&) override;
    void paintOverChildren (Graphics&) override;
    void resized() override;

    // ========================================================================
    void setMainContent (Component& mainContentToShow);
    void addNavButton (const String& name, const String& svg);
    void setNavPage (const String& name, Component& page);
    void setBackdrop (const String& name, Component& backdrop);
    void setBackdropRevealed (bool shouldBackdropBeRevealed);
    void toggleNavPagesRevealed();
    void toggleBackdropRevealed();

private:
    Geometry computeGeometry() const;
    void showSourceList();
    void hideSourceList();
    void updatePageVisibility();
    void layout();

    WeakReference<Component> mainContent;
    WeakReference<Component> lastNavButtonClosed;
    Array<std::unique_ptr<NavButton>> navButtons;
    std::unique_ptr<BackdropButton> backdropButton;

    bool sourceListVisible = false;
    int topNavHeight = 40;
    int leftNavWidth = 60;
    int sourceListWidth = 250;
};
