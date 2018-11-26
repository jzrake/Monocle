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
    setMultiSelectEnabled (false);
    setRootItemVisible (true);
}

void ExpressionEditor::addListener (Listener *listener)
{
    listeners.add (listener);
}

void ExpressionEditor::removeListener(Listener *listener)
{
    listeners.remove (listener);
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
    if (getNumSelectedItems() == 1)
    {
        if (key == KeyPress::returnKey)
        {
            return showEditorInSelectedItem();
        }
        if (key == KeyPress::backspaceKey)
        {
            return removeSelectedItem();
        }
    }
    return TreeView::keyPressed (key);
}

void ExpressionEditor::focusOfChildComponentChanged (FocusChangeType)
{
    if (hasKeyboardFocus (true) && getNumSelectedItems() == 0)
    {
        getRootItem()->setSelected (true, false);
    }
    repaint();
}

//==========================================================================
bool ExpressionEditor::showEditorInSelectedItem()
{
    dynamic_cast<ExpressionEditorItem*> (getSelectedItem(0))->label.showEditor();
    return true;
}

bool ExpressionEditor::removeSelectedItem()
{
    auto item = getSelectedItem(0);

    if (auto parent = item->getParentItem())
    {
        auto index = item->getIndexInParent();

        parent->removeSubItem (index);

        if (auto next = parent->getSubItem (index))
            next->setSelected (true, true);
        else if (auto next = parent->getSubItem (index - 1))
            next->setSelected (true, true);
        else
            parent->setSelected (true, true);

        sendNewExpression();
        return true;
    }
    return false;
}

void ExpressionEditor::sendNewExpression()
{
    listeners.call (&Listener::expressionEditorNewExpression, computeExpression());
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
        g.fillAll (getOwnerView()->hasKeyboardFocus (true) ? Colours::lightblue : Colours::lightgrey);
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

bool ExpressionEditorItem::isInterestedInDragSource (const DragAndDropTarget::SourceDetails& details)
{
    return mightContainSubItems();
}

void ExpressionEditorItem::itemDropped (const DragAndDropTarget::SourceDetails& details, int insertIndex)
{
    auto tree = dynamic_cast<ExpressionEditor*> (getOwnerView());
    auto part = crt::parser::parse (details.description.toString().getCharPointer());
    addSubItem (new ExpressionEditorItem (part), insertIndex);
    tree->sendNewExpression();
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
    auto tree = dynamic_cast<ExpressionEditor*> (getOwnerView());

    try {
        setExpression (crt::parser::parse (label.getText().getCharPointer()));
        tree->listeners.call (&ExpressionEditor::Listener::expressionEditorNewExpression, tree->computeExpression());
    }
    catch (const std::exception& e)
    {
        tree->listeners.call (&ExpressionEditor::Listener::expressionEditorParserError, e.what());
    }
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
