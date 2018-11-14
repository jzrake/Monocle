#pragma once
#include "JuceHeader.h"




// ============================================================================
class AppSkeleton : public Component
{
public:
    class NavButton;
    class BackdropButton;

    struct Geometry
    {
        Rectangle<int> topNav;
        Rectangle<int> leftNav;
        Rectangle<int> sourceList;
        Rectangle<int> mainContent;
        Rectangle<int> backdrop;
        Rectangle<int> backdropButton;
    };

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
    void toggleBackdropRevealed();

private:
    Geometry computeGeometry() const;
    void showSourceList();
    void hideSourceList();
    void updatePageVisibility();
    void layout();

    WeakReference<Component> mainContent;
    Array<std::unique_ptr<NavButton>> navButtons;
    std::unique_ptr<BackdropButton> backdropButton;

    bool sourceListVisible = false;
    int topNavHeight = 40;
    int leftNavWidth = 60;
    int sourceListWidth = 200;
};
