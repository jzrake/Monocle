#include "SymbolDetailsView.hpp"
#define MAX_CACHED_OPENNESS_STATES 100




//==============================================================================
SymbolDetailsView::SymbolItem::SymbolItem (const std::string& key, const mcl::Object& object)
: key (key)
, object (object)
{
    setDrawsInLeftMargin (true);

    int n = 0;

    if (object.type() == 'L')
        for (const auto& item : object.get<mcl::Object::List>())
            addSubItem (new SymbolItem ("[" + std::to_string (n++) + "]", item));

    if (object.type() == 'D')
        for (const auto& item : object.get<mcl::Object::Dict>())
            addSubItem (new SymbolItem (item.first, item.second));
}

bool SymbolDetailsView::SymbolItem::mightContainSubItems()
{
    return object.type() == 'L' || object.type() == 'D';
}

void SymbolDetailsView::SymbolItem::paintItem (Graphics& g, int width, int height)
{
    if (isSelected())
    {
        if (getOwnerView()->hasKeyboardFocus (true))
            g.fillAll (Colours::lightblue);
        else
            g.fillAll (Colours::lightgrey);
    }

    g.setColour (Colours::black);
    g.setFont (Font ("Monaco", 12, 0));

    std::string descr;

    if (false) {}
    else if (object.type() == 'n') descr = "None";
    else if (object.type() == 'b') descr = std::to_string (object.get<bool>());
    else if (object.type() == 'i') descr = std::to_string (object.get<int>());
    else if (object.type() == 'd') descr = std::to_string (object.get<double>());
    else if (object.type() == 'L') descr = "List";
    else if (object.type() == 'D') descr = "Dict";
    else if (object.type() == 'U') descr = object.get<mcl::Object::Data>().describe();
    else if (object.type() == 'F') descr = "Func";
    else if (object.type() == 'S') descr = object.get<std::string>();
    else                           descr = "Object";

    g.drawText (key + " = " + descr, 0, 0, width, height, Justification::centredLeft);
}

String SymbolDetailsView::SymbolItem::getUniqueName() const
{
    return String (getIndexInParent());
}

//==============================================================================
void SymbolDetailsView::SymbolItem::itemClicked (const MouseEvent& e)
{
    if (isSelected() && e.mods.isPopupMenu())
    {
//        PopupMenu menu;
//        menu.addItem (1, "Create Line Plot");
//        menu.showMenuAsync (PopupMenu::Options(), [this] (int code)
//        {
//            auto tree = dynamic_cast<SymbolDetailsView*> (getOwnerView());
//            tree->listeners.call (&Listener::symbolDetailsWantsNewDefinition, 1, tree->getSelectedKeys());
//        });
    }
}

void SymbolDetailsView::SymbolItem::itemSelectionChanged (bool isNowSelected)
{
    DBG(key << " selected ? " << int (isNowSelected));
}



//==============================================================================
SymbolDetailsView::SymbolDetailsView()
{
    setIndentSize (12);
    setMultiSelectEnabled (true);
    setRootItemVisible (false);
}

void SymbolDetailsView::addListener (Listener* listener)
{
    listeners.add (listener);
}

void SymbolDetailsView::removeListener(Listener* listener)
{
    listeners.remove (listener);
}

void SymbolDetailsView::setViewedObject (const std::string& key, const mcl::Object& objectToView)
{
    if (opennessStates.size() > MAX_CACHED_OPENNESS_STATES)
        opennessStates.clear();

    opennessStates[currentKey] = std::unique_ptr<XmlElement> (getOpennessState (true));
    currentKey = key;
    root = objectToView.empty() ? nullptr : std::make_unique<SymbolItem> (key, objectToView);

    setRootItem (root.get());

    if (opennessStates.find (currentKey) != opennessStates.end())
        if (opennessStates.at (currentKey))
            restoreOpennessState (*opennessStates.at (currentKey), true);
}

const std::string& SymbolDetailsView::getCurrentSymbol() const
{
    return currentKey;
}

StringArray SymbolDetailsView::getSelectedKeys() const
{
    StringArray res;
    res.ensureStorageAllocated (getNumSelectedItems());

    for (int n = 0; n < getNumSelectedItems(); ++n)
        res.add (dynamic_cast<SymbolItem*> (getSelectedItem (n))->key);
    return res;
}

//==============================================================================
void SymbolDetailsView::focusGained (FocusChangeType)
{
    repaint();
}

void SymbolDetailsView::focusLost (FocusChangeType)
{
    repaint();
}

void SymbolDetailsView::focusOfChildComponentChanged (FocusChangeType)
{
    repaint();
}
