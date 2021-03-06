#pragma once
#include "JuceHeader.h"




//==============================================================================
class DualComponentView : public Component
{
public:
    DualComponentView();
    void setContent1 (Component& contentFor1);
    void setContent2 (Component& contentFor2);

    // ========================================================================
    void resized() override;
    bool hitTest (int x, int y) override;
    void mouseDown (const MouseEvent&) override;
    void mouseDrag (const MouseEvent&) override;

private:
    void resetContent();
    void layout();

    // ========================================================================
    class Resizer : public Component
    {
    public:
        void paint (Graphics&) override;
        MouseCursor getMouseCursor() override;
    };

    WeakReference<Component> content1;
    WeakReference<Component> content2;
    Resizer resizer;
    int bottomHeight = 300;
    int bottomHeightAtMouseDown = 0;
};




// ============================================================================
class AppSkeleton : public Component
{
public:
    class NavButton;
    class BackdropButton;
    class TopNavComponent;

    // ========================================================================
    struct Geometry
    {
        Rectangle<int> topNav;
        Rectangle<int> leftNav;
        Rectangle<int> sourceList;
        Rectangle<int> mainContent;
        Rectangle<int> backdrop;
        Rectangle<int> backdropButton;
        Rectangle<int> verticalGap;
    };

    // ========================================================================
    AppSkeleton();
    ~AppSkeleton();

    // ========================================================================
    void setMainContent (Component& mainContentToShow);
    void addNavButton (const String& name, const String& svg);
    void setNavPage (const String& name, Component& page);
    void setBackdrop (const String& name, Component& backdrop);
    void setBackdropRevealed (bool shouldBackdropBeRevealed);
    void toggleNavPagesRevealed();
    void toggleBackdropRevealed();
    void openNavSection (const String& name);
    String getCurrentNavSectionName() const;

    // ========================================================================
    void paint (Graphics&) override;
    void paintOverChildren (Graphics&) override;
    void resized() override;
    void mouseEnter (const MouseEvent&) override;
    void mouseExit (const MouseEvent&) override;
    void mouseMove (const MouseEvent&) override;
    void mouseDown (const MouseEvent&) override;
    void mouseDrag (const MouseEvent&) override;
    MouseCursor getMouseCursor() override;

private:
    Geometry computeGeometry() const;
    bool isMouseAtRightEdgeOfSourceList (const Point<float>& p) const;
    void showSourceList();
    void hideSourceList();
    void updatePageVisibility();
    void layout (bool animate=false);

    WeakReference<Component> mainContent;
    WeakReference<Component> lastNavButtonClosed;
    Array<std::unique_ptr<NavButton>> navButtons;
    std::unique_ptr<BackdropButton> backdropButton;
    std::unique_ptr<TopNavComponent> topNavComponent;

    bool sourceListVisible = false;
    bool mouseIsAtRightEdgeOfSourceList = false;
    int topNavHeight    = 40;
    int leftNavWidth    = 60;
    int sourceListWidth = 250;
    int backdropHeight  = 200;
    int sourceListWidthAtMouseDown = 0;
};
