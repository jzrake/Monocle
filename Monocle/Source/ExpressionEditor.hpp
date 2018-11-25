#pragma once
#include "JuceHeader.h"
#include "3rdParty/crt-kernel/kernel.hpp"




//==============================================================================
class ExpressionEditor;
class ExpressionEditorItem;
class ExpressionEditorItemView;




//==============================================================================
class ExpressionEditor : public TreeView
{
public:
    //==========================================================================
    ExpressionEditor();
    void setExpression (crt::expression expr);
    crt::expression computeExpression() const;

    //==========================================================================
    bool keyPressed (const KeyPress& key) override;
private:
    std::unique_ptr<ExpressionEditorItem> root;
};




//==============================================================================
class ExpressionEditorItem : public TreeViewItem, private Label::Listener
{
public:
    //==========================================================================
    ExpressionEditorItem (crt::expression expr);
    void setExpression (crt::expression expr);
    crt::expression computeExpression() const;

    //==========================================================================
    void paintItem (Graphics& g, int width, int height) override;
    Component* createItemComponent() override;
    String getUniqueName() const override;
    bool mightContainSubItems() override;
    void itemClicked (const MouseEvent&) override;
    void itemDoubleClicked (const MouseEvent&) override;
    void itemSelectionChanged (bool isNowSelected) override;

private:
    //==========================================================================
    void labelTextChanged (Label* labelThatHasChanged) override;

    //==========================================================================
    friend class ExpressionEditor;
    friend class ExpressionEditorItemView;
    crt::expression expr;
    Label label;
};




//==============================================================================
class ExpressionEditorItemView : public Component
{
public:
    //==========================================================================
    ExpressionEditorItemView (ExpressionEditorItem& item);

    //==========================================================================
    void resized() override;
    void paint (Graphics& g) override;
private:
    ExpressionEditorItem& item;
};
