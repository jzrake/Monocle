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

    SafePointer<Component> mainContent;
    std::vector<std::unique_ptr<Button>> navButtons;

    int topNavHeight = 40;
    int leftNavWidth = 60;
    int sourceListWidth = 200;
};
