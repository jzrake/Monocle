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
        virtual void kernelEditorSelectionChanged() = 0;
        virtual void kernelEditorRulePunched (const std::string& key) = 0;
    };

    KernelEditor();
    void addListener (Listener* listener);
    void removeListener (Listener* listener);
    void setKernel (const Kernel* kernelToView);
    StringArray getSelectedRules();

    //==========================================================================
    bool keyPressed (const KeyPress& key) override;
    void focusOfChildComponentChanged (FocusChangeType) override;

private:
    //==========================================================================
    bool showEditorInSelectedItem();
    void sendSelectionChanged();
    bool sendRulePunched();

    friend class KernelEditorItem;
    ListenerList<Listener> listeners;
    std::unique_ptr<KernelEditorItem> root;
    const Kernel* kernel = nullptr;
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
    crt::expression getExpressionToIndexInParent() const;

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
