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
    class Listener
    {
    public:
        virtual ~Listener() {}
        virtual void expressionEditorNewExpression (const crt::expression&) = 0;
        virtual void expressionEditorEncounteredError (const std::string&) = 0;
    };

    //==========================================================================
    ExpressionEditor();
    void addListener (Listener* listener);
    void removeListener (Listener* listener);
    void setExpression (crt::expression expr);
    crt::expression computeExpression() const;

    //==========================================================================
    bool keyPressed (const KeyPress& key) override;
    void focusOfChildComponentChanged (FocusChangeType cause) override;

private:
    //==========================================================================
    bool showEditorInSelectedItem();
    bool removeSelectedItem();
    void sendNewExpression();

    //==========================================================================
    friend class ExpressionEditorItem;
    ListenerList<Listener> listeners;
    std::unique_ptr<ExpressionEditorItem> root;
};




//==============================================================================
class ExpressionEditorItem : public TreeViewItem, private Label::Listener
{
public:
    //==========================================================================
    ExpressionEditorItem (crt::expression expr);
    void setExpression (crt::expression expr);
    void insertPart (int index, crt::expression part);
    crt::expression computeExpression() const;

    //==========================================================================
    void paintItem (Graphics& g, int width, int height) override;
    Component* createItemComponent() override;
    String getUniqueName() const override;
    bool mightContainSubItems() override;
    bool isInterestedInDragSource (const DragAndDropTarget::SourceDetails& dragSourceDetails) override;
    void itemDropped (const DragAndDropTarget::SourceDetails& dragSourceDetails, int insertIndex) override;
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
