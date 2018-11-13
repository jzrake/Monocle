#pragma once
#include "JuceHeader.h"




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
    void setMainContent (Component* mainContentToShow);

private:
    Geometry computeGeometry() const;
    void showSourceList();
    void hideSourceList();
    void layout();

    SafePointer<Component> mainContent;
    std::vector<std::unique_ptr<Button>> navButtons;

    bool sourceListVisible = false;

    int topNavHeight = 40;
    int leftNavWidth = 60;
    int sourceListWidth = 200;

    friend class NavButton;
};
