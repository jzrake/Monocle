#pragma once
#include "JuceHeader.h"
#include "MaterialIcons.hpp"




// ============================================================================
class AppSkeleton : public Component
{
public:
    class NavButton;

    struct Geometry
    {
        Rectangle<int> topNav;
        Rectangle<int> leftNav;
        Rectangle<int> sourceList;
        Rectangle<int> mainContent;
    };

    AppSkeleton();
    void paint (Graphics&) override;
    void resized() override;

    // ========================================================================
    void setMainContent (Component& mainContentToShow);
    void addNavButton (const String& name, const String& svg);
    void setNavPage (const String& name, Component& page);

private:
    Geometry computeGeometry() const;
    void showSourceList();
    void hideSourceList();
    void updatePageVisibility();
    void layout();

    WeakReference<Component> mainContent;
    Array<std::unique_ptr<Button>> navButtons;

    bool sourceListVisible = false;

    int topNavHeight = 40;
    int leftNavWidth = 60;
    int sourceListWidth = 200;
};
