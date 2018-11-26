#include "KernelEditor.hpp"




// ============================================================================
KernelEditor::KernelEditor()
{
    kernel.insert ("list", var::NativeFunction (Runtime::builtin_list));
    kernel.insert ("dict", var::NativeFunction (Runtime::builtin_dict));
    kernel.insert ("data", JSON::fromString ("[1, 2, 3]"));

    DynamicObject* kernelObject = new DynamicObject;

    for (const auto& rule : kernel)
    {
        kernelObject->setProperty (String (rule.first), rule.second.value);
    }

    root = std::make_unique<KernelEditorItem> (var(), kernelObject);
    setRootItem (root.get());
    setIndentSize (12);
    setMultiSelectEnabled (true);
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

//==============================================================================
bool KernelEditor::keyPressed (const KeyPress& key)
{
    if (key == KeyPress::returnKey && getNumSelectedItems() == 1)
    {
        dynamic_cast<KernelEditorItem*> (getSelectedItem(0))->label.showEditor();
        return true;
    }
    return TreeView::keyPressed (key);
}




//==========================================================================
KernelEditorItem::KernelEditorItem (const var& key, const var& value) : key (key), value (value)
{
    label.addListener (this);
    setDrawsInLeftMargin (true);

    if (value.getDynamicObject())
    {
        for (const auto& property : value.getDynamicObject()->getProperties())
        {
            addSubItem (new KernelEditorItem (property.name.toString(), property.value));
        }
    }
    else if (value.getArray())
    {
        int index = 0;

        for (const auto& item : *value.getArray())
        {
            addSubItem (new KernelEditorItem (index++, item));
        }
    }
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
        g.fillAll (Colours::lightblue);
    }
}

Component* KernelEditorItem::createItemComponent()
{
    return new KernelEditorItemView (*this);
}

String KernelEditorItem::getUniqueName() const
{
    return std::to_string (getIndexInParent());
}

var KernelEditorItem::getDragSourceDescription()
{
    if (getParentItem() == getOwnerView()->getRootItem())
    {
        return key;
    }
    return var();
}

bool KernelEditorItem::mightContainSubItems()
{
    return value.isArray() || value.isObject();
}

void KernelEditorItem::itemClicked (const MouseEvent&)
{
}

void KernelEditorItem::itemDoubleClicked (const MouseEvent&)
{
    label.showEditor();
}
void KernelEditorItem::itemSelectionChanged (bool isNowSelected)
{
}

//==========================================================================
void KernelEditorItem::labelTextChanged (Label* labelThatHasChanged)
{
    // auto tree = dynamic_cast<KernelEditor*>(getOwnerView());

    try {
    }
    catch (const std::exception& e)
    {
    }
}




//==========================================================================
KernelEditorItemView::KernelEditorItemView (KernelEditorItem& item) : item (item)
{
    item.label.setText (item.key.toString(), NotificationType::dontSendNotification);
    item.label.setColour (Label::textColourId, Colours::black);
    item.label.setFont (Font ("Monaco", 12, 0));
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
