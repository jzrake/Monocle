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
        virtual void kernelEditorWantsNewRule (const crt::expression&) = 0;
        virtual void kernelEditorWantsRuleRemoved (const std::string&) = 0;
        virtual void kernelEditorWantsRuleRelabeled (const std::string& from, const std::string& to) = 0;
    };

    KernelEditor();
    void addListener (Listener* listener);
    void removeListener (Listener* listener);
    void setKernel (const Kernel* kernelToView);
    void selectRule (const std::string& key);
    void selectNext();
    void setEmphasizedKey (const std::string& keyToEmphasize);
    void createRule();
    StringArray getSelectedRules();
    std::string getEmphasizedKey();

    //==========================================================================
    bool keyPressed (const KeyPress& key) override;
    void focusOfChildComponentChanged (FocusChangeType) override;

private:
    //==========================================================================
    struct ItemComparator;

    //==========================================================================
    bool showEditorInSelectedItem();
    void sendSelectionChanged();
    bool sendRulePunched();
    bool removeSelectedRules();
    bool relabelSelectedRule(const std::string& from, const std::string& to);

    friend class KernelEditorItem;
    ListenerList<Listener> listeners;
    std::unique_ptr<KernelEditorItem> root;
    std::unique_ptr<XmlElement> state;
    Font font;
    const Kernel* kernel = nullptr;
    bool creatingNewRule = false;
    std::string emphasizedKey;
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
    void ownerViewChanged (TreeView* newOwner) override;

private:
    //==========================================================================
    void labelTextChanged (Label* labelThatHasChanged) override;

    //==========================================================================
    void setFontForEmphasizedKey();
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
