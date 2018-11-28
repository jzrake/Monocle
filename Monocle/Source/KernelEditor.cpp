#include "KernelEditor.hpp"
#include "Runtime.hpp"




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
    getViewport()->setWantsKeyboardFocus (false);
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
    setRootItem (nullptr);
    setRootItem ((root = std::make_unique<KernelEditorItem> (*this, var(), kernelObject)).get());

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
        auto item = root->getKernelEditorSubItem(n);

        if (item->stringKey == key)
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
        auto item = root->getKernelEditorSubItem(n);
        item->setFontForEmphasizedKey();
    }
}

void KernelEditor::createRule()
{
    auto item = new KernelEditorItem (*this, "new-rule", var());
    root->addSubItem (item);
    item->setSelected (true, true);
    creatingNewRule = true;
    listeners.call (&Listener::kernelEditorWantsNewRule, crt::expression().keyed ("new-rule"));
}

KernelEditorItem* KernelEditor::getSoleSelectedItem()
{
    if (getNumSelectedItems() == 1)
    {
        return dynamic_cast<KernelEditorItem*> (getSelectedItem(0));
    }
    return nullptr;
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
void KernelEditor::sendRelabelSelectedRule (const std::string &from, const std::string &to)
{
    auto item = getSoleSelectedItem();
    jassert (item != nullptr);
    jassert (item->stringKey == from);
    listeners.call (&Listener::kernelEditorWantsRuleRelabeled, from, to);
}

void KernelEditor::sendSelectionChanged()
{
    listeners.call (&Listener::kernelEditorSelectionChanged);
}

bool KernelEditor::showEditorInSelectedItem()
{
    if (auto item = getSoleSelectedItem())
    {
        if (item->isAtKernelLevel() && ! item->isLocked())
        {
            item->label.showEditor();
            return true;
        }
    }
    return false;
}

bool KernelEditor::sendRulePunched()
{
    if (auto item = getSoleSelectedItem())
    {
        if (item->isAtKernelLevel() && ! item->isLiteral())
        {
            listeners.call (&Listener::kernelEditorRulePunched, item->stringKey);
            return true;
        }
    }
    return false;
}

bool KernelEditor::removeSelectedRules()
{
    if (auto item = getSoleSelectedItem())
    {
        if (item->isAtKernelLevel() && ! item->isLocked())
        {
            listeners.call (&Listener::kernelEditorWantsRuleRemoved, item->stringKey);
            return true;
        }
    }
    return false;
}




//==========================================================================
KernelEditorItem::KernelEditorItem (KernelEditor& tree, const var& key, const var& value)
: tree (tree)
, key (key)
, value (value)
{
    KernelEditor::ItemComparator comparator;
    stringKey = key.toString().toStdString();

    label.addListener (this);
    label.setText (key.toString(), NotificationType::dontSendNotification);
    label.setColour (Label::textColourId, Colours::black);

    setDrawsInLeftMargin (true);
    int index = 0;

    if (key.isVoid()) /* A void key is used for the root node. */
        for (const auto& property : value.getDynamicObject()->getProperties())
            addSubItemSorted (comparator, new KernelEditorItem (tree, property.name.toString(), property.value));

    else if (value.getDynamicObject())
        for (const auto& property : value.getDynamicObject()->getProperties())
            addSubItem (new KernelEditorItem (tree, property.name.toString(), property.value));

    else if (value.getArray())
        for (const auto& item : *value.getArray())
            addSubItem (new KernelEditorItem (tree, index++, item));
}

KernelEditorItem* KernelEditorItem::getKernelEditorSubItem (int index)
{
    return dynamic_cast<KernelEditorItem*> (getSubItem(index));
}

KernelEditorItem* KernelEditorItem::getKernelEditorParentItem()
{
    return dynamic_cast<KernelEditorItem*> (getParentItem());
}

const KernelEditorItem* KernelEditorItem::getKernelEditorSubItem (int index) const
{
    return dynamic_cast<const KernelEditorItem*> (getSubItem(index));
}

const KernelEditorItem* KernelEditorItem::getKernelEditorParentItem() const
{
    return dynamic_cast<const KernelEditorItem*> (getParentItem());
}

void KernelEditorItem::setValue (const var& newValue)
{
    jassertfalse;
}

bool KernelEditorItem::isAtKernelLevel() const
{
    return getParentItem() == tree.getRootItem();
}

bool KernelEditorItem::isLocked() const
{
    return ! isAtKernelLevel() || (tree.kernel->flags_at (stringKey) & Runtime::locked);
}

bool KernelEditorItem::isLiteral() const
{
    return isAtKernelLevel()
    && ! tree.kernel->at (stringKey).isVoid()
    && tree.kernel->expr_at (stringKey).empty();
}

//==========================================================================
void KernelEditorItem::paintItem (Graphics& g, int width, int height)
{
    if (isSelected())
    {
        g.fillAll (tree.hasKeyboardFocus (true) ? Colours::lightblue : Colours::lightgrey);
    }

    if (isAtKernelLevel())
    {
        if (! tree.kernel->error_at (stringKey).empty())
        {
            g.setFont (tree.font.withHeight (10));
            g.setColour (Colours::red);
            g.drawText (tree.kernel->error_at (stringKey), 0, 0, width - 10, height, Justification::centredRight);
        }
    }

    g.setFont (tree.font.withHeight (10));
    g.setColour (Colours::grey);
    g.drawText (Runtime::summarize (value), 0, 0, width - 10, height, Justification::centredRight);
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
    return value.isArray() || value.getDynamicObject();
}

void KernelEditorItem::itemClicked (const MouseEvent& e)
{
    if (e.mods.isPopupMenu())
    {
        PopupMenu menu;
        menu.addItem (1, "Create Rule");
        menu.addItem (2, "Delete Rule",  isAtKernelLevel() && ! isLocked());
        menu.addItem (3, "Relabel Rule", isAtKernelLevel() && ! isLocked());
        menu.addItem (4, "Edit Rule",    isAtKernelLevel() && ! isLocked() && ! isLiteral());

        menu.showMenuAsync (PopupMenu::Options(), [this] (int code)
        {
            switch (code)
            {
                case 1: tree.createRule(); break;
                case 2: tree.removeSelectedRules(); break;
                case 3: tree.showEditorInSelectedItem(); break;
                case 4: tree.sendRulePunched(); break;
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
        tree.sendSelectionChanged();
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
        label.setText (key.toString(), NotificationType::dontSendNotification);
    }
    try {
        tree.sendRelabelSelectedRule (key.toString().toStdString(), label.getText().toStdString());
    }
    catch (const std::exception& e)
    {
        label.setText (key.toString(), NotificationType::dontSendNotification);
        tree.listeners.call (&KernelEditor::Listener::kernelEditorEncounteredError, e.what());
    }
}

//==========================================================================
crt::expression KernelEditorItem::getExpressionToIndexInParent() const
{
    if (auto parent = getKernelEditorParentItem())
    {
        if (isAtKernelLevel())
            return crt::expression::symbol (stringKey);
        else if (key.isInt())
            return {crt::expression::symbol ("item"), parent->getExpressionToIndexInParent(), int (key)};
        else if (key.isString())
            return {crt::expression::symbol ("attr"), parent->getExpressionToIndexInParent(), stringKey};
        else
            jassertfalse;
    }
    return crt::expression{};
}

void KernelEditorItem::setFontForEmphasizedKey()
{
    label.setFont (key == tree.emphasizedKey ? tree.font.italicised() : tree.font);
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
