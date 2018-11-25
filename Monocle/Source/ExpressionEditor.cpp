#include "ExpressionEditor.hpp"




//==============================================================================
ExpressionEditor::ExpressionEditor()
{
    crt::expression e {
        1,
        2.3,
        std::string("sdf"),
        crt::expression::symbol("a"),
        {1, crt::expression::symbol("b"), crt::expression::symbol("b")}};

    root = std::make_unique<ExpressionEditorItem>(e);
    setRootItem (root.get());
    setIndentSize (12);
    setMultiSelectEnabled (true);
    setRootItemVisible (true);
}

void ExpressionEditor::setExpression (crt::expression expr)
{
    auto state = std::unique_ptr<XmlElement> (getOpennessState (true));
    root = std::make_unique<ExpressionEditorItem> (expr);
    setRootItem (root.get());
    restoreOpennessState (*state, true);
}

crt::expression ExpressionEditor::computeExpression() const
{
    return root->computeExpression();
}

//==============================================================================
bool ExpressionEditor::keyPressed (const KeyPress& key)
{
    if (key == KeyPress::returnKey && getNumSelectedItems() == 1)
    {
        dynamic_cast<ExpressionEditorItem*> (getSelectedItem(0))->label.showEditor();
        return true;
    }
    return TreeView::keyPressed (key);
}




//==========================================================================
ExpressionEditorItem::ExpressionEditorItem (crt::expression expr) : expr (expr)
{
    label.addListener (this);
    setDrawsInLeftMargin (true);

    for (const auto& e : expr)
    {
        addSubItem (new ExpressionEditorItem (e));
    }
}

void ExpressionEditorItem::setExpression (crt::expression newExpr)
{
    expr = newExpr;
    clearSubItems();

    for (const auto& e : expr)
    {
        addSubItem (new ExpressionEditorItem (e));
    }
}

crt::expression ExpressionEditorItem::computeExpression() const
{
    if (expr.dtype() != crt::data_type::composite)
    {
        return expr;
    }
    std::vector<crt::expression> parts;

    for (int n = 0; n < getNumSubItems(); ++n)
    {
        auto item = dynamic_cast<ExpressionEditorItem*> (getSubItem(n));
        parts.push_back (item->computeExpression());
    }
    return parts;
}

//==========================================================================
void ExpressionEditorItem::paintItem (Graphics& g, int width, int height)
{
    if (isSelected())
    {
        g.fillAll (Colours::lightblue);
    }
}

Component* ExpressionEditorItem::createItemComponent()
{
    return new ExpressionEditorItemView (*this);
}

String ExpressionEditorItem::getUniqueName() const
{
    return std::to_string (getIndexInParent());
}

bool ExpressionEditorItem::mightContainSubItems()
{
    return expr.dtype() == crt::data_type::composite;
}

void ExpressionEditorItem::itemClicked (const MouseEvent&)
{
}

void ExpressionEditorItem::itemDoubleClicked (const MouseEvent&)
{
    label.showEditor();
}
void ExpressionEditorItem::itemSelectionChanged (bool isNowSelected)
{
}

//==========================================================================
void ExpressionEditorItem::labelTextChanged (Label* labelThatHasChanged)
{
    try {
        setExpression (crt::parser::parse (label.getText().getCharPointer()));
    }
    catch (const std::exception& e)
    {
         DBG(e.what());
    }
    auto tree = dynamic_cast<ExpressionEditor*>(getOwnerView());
    tree->setExpression (tree->computeExpression());
}




//==========================================================================
ExpressionEditorItemView::ExpressionEditorItemView (ExpressionEditorItem& item) : item (item)
{
    item.label.setText (item.expr.str(), NotificationType::dontSendNotification);
    item.label.setColour (Label::textColourId, Colours::black);
    item.label.setFont (Font ("Monaco", 12, 0));
    setInterceptsMouseClicks (false, false);
    addAndMakeVisible (item.label);
}

void ExpressionEditorItemView::resized()
{
    item.label.setBounds (getLocalBounds());
}

void ExpressionEditorItemView::paint (Graphics& g)
{
}
