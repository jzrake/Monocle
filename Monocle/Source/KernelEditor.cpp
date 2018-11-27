#include "KernelEditor.hpp"




// ============================================================================
struct KernelEditor::ItemComparator
{
    using ElementType = TreeViewItem*;
    int compareElements (ElementType first, ElementType second)
    {
        auto v1 = dynamic_cast<KernelEditorItem*> (first)->key;
        auto v2 = dynamic_cast<KernelEditorItem*> (second)->key;
        return v1.toString().compare (v2.toString());
    }
};




// ============================================================================
KernelEditor::KernelEditor()
{
    font = Font ("Monaco", 12, 0);
    setIndentSize (12);
    setMultiSelectEnabled (false);
    setRootItemVisible (false);
}

void KernelEditor::addListener (Listener *listener)
{
    listeners.add (listener);
}

void KernelEditor::removeListener(Listener *listener)
{
    listeners.remove (listener);
}

void KernelEditor::setKernel (const Kernel* kernelToView)
{
    state.reset (getOpennessState (true));

    kernel = kernelToView;
    DynamicObject* kernelObject = new DynamicObject;

    for (const auto& rule : *kernel)
    {
        kernelObject->setProperty (String (rule.first), rule.second.value);
    }
    root = std::make_unique<KernelEditorItem> (var(), kernelObject);

    setRootItem (root.get());

    if (state)
    {
        restoreOpennessState (*state, true);
    }

    if (creatingNewRule)
    {
        creatingNewRule = false;
        showEditorInSelectedItem();
    }
}

void KernelEditor::selectRule (const std::string& key)
{
    for (int n = 0; n < root->getNumSubItems(); ++n)
    {
        auto item = dynamic_cast<KernelEditorItem*> (root->getSubItem(n));

        if (item->key == String (key))
        {
            item->setSelected (true, true);
            return;
        }
    }
}

void KernelEditor::selectNext()
{
    if (getNumSelectedItems() == 1)
    {
        auto item = getSelectedItem(0);
        auto index = item->getIndexInParent();

        if (auto parent = item->getParentItem())
        {
            if (auto next = parent->getSubItem (index + 1))
                next->setSelected (true, true);
            else if (auto next = parent->getSubItem (index - 1))
                next->setSelected (true, true);
            else
                parent->setSelected (true, true);
        }
    }
}

void KernelEditor::setEmphasizedKey (const std::string &keyToEmphasize)
{
    emphasizedKey = keyToEmphasize;

    for (int n = 0; n < root->getNumSubItems(); ++n)
    {
        auto item = dynamic_cast<KernelEditorItem*> (root->getSubItem(n));
        item->setFontForEmphasizedKey();
    }
}

void KernelEditor::createRule()
{
    auto item = new KernelEditorItem ("new-rule", var());
    root->addSubItem (item);
    item->setSelected (true, true);
    creatingNewRule = true;
    listeners.call (&Listener::kernelEditorWantsNewRule, crt::expression().keyed ("new-rule"));
}

StringArray KernelEditor::getSelectedRules()
{
    StringArray res;

    for (int n = 0; n < getNumSelectedItems(); ++n)
    {
        res.add (dynamic_cast<KernelEditorItem*> (getSelectedItem(n))->key);
    }
    return res;
}

std::string KernelEditor::getEmphasizedKey()
{
    return emphasizedKey;
}

//==============================================================================
bool KernelEditor::keyPressed (const KeyPress& key)
{
    if (key == KeyPress::returnKey && getNumSelectedItems() == 1)
    {
        return showEditorInSelectedItem();
    }
    if (key == KeyPress::spaceKey && getNumSelectedItems() == 1)
    {
        return sendRulePunched();
    }
    if (key == KeyPress::backspaceKey && getNumSelectedItems() == 1)
    {
        return removeSelectedRules();
    }
    return TreeView::keyPressed (key);
}

void KernelEditor::focusOfChildComponentChanged (FocusChangeType)
{
    if (hasKeyboardFocus (true) && getNumSelectedItems() == 0)
    {
        getRootItem()->setSelected (true, false);
    }
    repaint();
}

//==========================================================================
bool KernelEditor::showEditorInSelectedItem()
{
    if (auto item = dynamic_cast<KernelEditorItem*> (getSelectedItem(0)))
    {
        if (item->getParentItem() == root.get())
        {
            item->label.showEditor();
            return true;
        }
    }
    return false;
}

void KernelEditor::sendSelectionChanged()
{
    listeners.call (&Listener::kernelEditorSelectionChanged);
}

bool KernelEditor::sendRulePunched()
{
    if (getNumSelectedItems() == 1)
    {
        if (getSelectedItem(0)->getParentItem() == root.get())
        {
            auto key = dynamic_cast<KernelEditorItem*> (getSelectedItem(0))->key.toString();
            listeners.call (&Listener::kernelEditorRulePunched, key.toStdString());
            return true;
        }
    }
    return false;
}

bool KernelEditor::removeSelectedRules()
{
    if (getNumSelectedItems() == 1)
    {
        if (getSelectedItem(0)->getParentItem() == root.get())
        {
            auto key = dynamic_cast<KernelEditorItem*> (getSelectedItem(0))->key.toString();
            listeners.call (&Listener::kernelEditorWantsRuleRemoved, key.toStdString());
            return true;
        }
    }
    return false;
}




//==========================================================================
KernelEditorItem::KernelEditorItem (const var& key, const var& value) : key (key), value (value)
{
    KernelEditor::ItemComparator comparator;

    label.addListener (this);
    label.setText (key.toString(), NotificationType::dontSendNotification);
    label.setColour (Label::textColourId, Colours::black);

    setDrawsInLeftMargin (true);
    int index = 0;

    if (key.isVoid()) /* A void key is used for the root node. */
        for (const auto& property : value.getDynamicObject()->getProperties())
            addSubItemSorted (comparator, new KernelEditorItem (property.name.toString(), property.value));

    else if (value.getDynamicObject())
        for (const auto& property : value.getDynamicObject()->getProperties())
            addSubItem (new KernelEditorItem (property.name.toString(), property.value));

    else if (value.getArray())
        for (const auto& item : *value.getArray())
            addSubItem (new KernelEditorItem (index++, item));
}

void KernelEditorItem::setValue (const var& newValue)
{
    jassertfalse;
}

//==========================================================================
void KernelEditorItem::paintItem (Graphics& g, int width, int height)
{
    if (isSelected())
    {
        g.fillAll (getOwnerView()->hasKeyboardFocus (true) ? Colours::lightblue : Colours::lightgrey);
    }
}

Component* KernelEditorItem::createItemComponent()
{
    return new KernelEditorItemView (*this);
}

String KernelEditorItem::getUniqueName() const
{
    return key.isVoid() ? "root" : key.toString();
}

var KernelEditorItem::getDragSourceDescription()
{
    return String (getExpressionToIndexInParent().str());
}

bool KernelEditorItem::mightContainSubItems()
{
    return value.isArray() || value.isObject();
}

void KernelEditorItem::itemClicked (const MouseEvent& e)
{
    if (e.mods.isPopupMenu()) 
    {
        PopupMenu menu;
        menu.addItem (1, "Create Rule");
        menu.addItem (2, "Delete Rule");

        menu.showMenuAsync (PopupMenu::Options(), [this] (int code)
        {
            auto tree = dynamic_cast<KernelEditor*> (getOwnerView());

            switch (code)
            {
                case 1: tree->createRule(); break;
                case 2: tree->removeSelectedRules(); break;
                default: break;
            }
        });
    }
}

void KernelEditorItem::itemDoubleClicked (const MouseEvent&)
{
}

void KernelEditorItem::itemSelectionChanged (bool isNowSelected)
{
    if (isNowSelected)
    {
        dynamic_cast<KernelEditor*> (getOwnerView())->sendSelectionChanged();
    }
}

void KernelEditorItem::ownerViewChanged (TreeView* newOwner)
{
    if (newOwner)
    {
        setFontForEmphasizedKey();
    }
}

//==========================================================================
void KernelEditorItem::labelTextChanged (Label*)
{
    if (label.getText().isEmpty())
    {
        label.setText ("none", NotificationType::dontSendNotification);
    }
    auto tree = dynamic_cast<KernelEditor*> (getOwnerView());
    tree->listeners.call (&KernelEditor::Listener::kernelEditorWantsRuleRenamed,
                          key.toString().toStdString(),
                          label.getText().toStdString());
    tree->sendRulePunched();
}

//==========================================================================
crt::expression KernelEditorItem::getExpressionToIndexInParent() const
{
    if (auto parent = dynamic_cast<KernelEditorItem*> (getParentItem()))
    {
        if (parent == getOwnerView()->getRootItem())
            return crt::expression::symbol (key.toString().toStdString());

        else if (key.isInt())
            return {crt::expression::symbol ("item"),
                parent->getExpressionToIndexInParent(), int (key)};

        else if (key.isString())
            return {crt::expression::symbol ("attr"), key.toString().toStdString(),
                parent->getExpressionToIndexInParent()};

        else
            jassertfalse;
    }
    return crt::expression{};
}

void KernelEditorItem::setFontForEmphasizedKey()
{
    auto tree = dynamic_cast<KernelEditor*> (getOwnerView());
    label.setFont (key == tree->emphasizedKey ? tree->font.italicised() : tree->font);
}




//==========================================================================
KernelEditorItemView::KernelEditorItemView (KernelEditorItem& item) : item (item)
{
    setInterceptsMouseClicks (false, false);
    addAndMakeVisible (item.label);
}

void KernelEditorItemView::resized()
{
    item.label.setBounds (getLocalBounds());
}

void KernelEditorItemView::paint (Graphics& g)
{
}
