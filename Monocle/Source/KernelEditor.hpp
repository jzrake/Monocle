#pragma once
#include "JuceHeader.h"
#include "Runtime.hpp"




//==============================================================================
class KernelEditor;
class KernelEditorItem;
class KernelEditorItemView;




//==============================================================================
class KernelEditor : public TreeView
{
public:
    class Listener
    {
    public:
        virtual ~Listener() {}
    };

    KernelEditor();
    void addListener (Listener* listener);
    void removeListener (Listener* listener);

    //==========================================================================
    bool keyPressed (const KeyPress& key) override;

private:
    ListenerList<Listener> listeners;
    std::unique_ptr<KernelEditorItem> root;
    Kernel kernel;
};





//==============================================================================
class KernelEditorItem : public TreeViewItem, private Label::Listener
{
public:
    //==========================================================================
    KernelEditorItem (const var& key, const var& value);
    void setValue (const var& newValue);

    //==========================================================================
    void paintItem (Graphics& g, int width, int height) override;
    Component* createItemComponent() override;
    String getUniqueName() const override;
    var getDragSourceDescription() override;
    bool mightContainSubItems() override;
    void itemClicked (const MouseEvent&) override;
    void itemDoubleClicked (const MouseEvent&) override;
    void itemSelectionChanged (bool isNowSelected) override;

private:
    //==========================================================================
    void labelTextChanged (Label* labelThatHasChanged) override;

    //==========================================================================
    friend class KernelEditor;
    friend class KernelEditorItemView;
    var key;
    var value;
    Label label;
};




//==============================================================================
class KernelEditorItemView : public Component
{
public:
    //==========================================================================
    KernelEditorItemView (KernelEditorItem& item);

    //==========================================================================
    void resized() override;
    void paint (Graphics& g) override;
private:
    KernelEditorItem& item;
};
