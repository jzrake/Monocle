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

String SymbolDetailsView::SymbolItem::getExpressionForPathInSymbol() const
{
    struct PathComponent
    {
        PathComponent (const std::string& key) : key (key) {}
        PathComponent (int index) : index (index) {}
        std::string key;
        int index = -1;
    };

    std::vector<PathComponent> components;
    auto item = this;

    while (item)
    {
        if (item->key.front() == '[' && item->key.back() == ']')
        {
            components.push_back (atoi (item->key.substr (1, item->key.size() - 2).data()));
        }
        else
        {
            components.push_back (item->key);
        }
        item = dynamic_cast<SymbolItem*> (item->getParentItem());
    }
    String expr = "()";

    for (const auto& c : components)
    {
        if (c.key == components.back().key)
        {
            expr = expr.replace ("()", c.key);
        }
        else if (c.key.empty())
        {
            expr = expr.replace ("()", "(item () " + String (c.index) + ")");
        }
        else
        {
            expr = expr.replace ("()", "(attr () '" + c.key + "')");
        }
    }
    return expr;
}

//==============================================================================
String SymbolDetailsView::SymbolItem::getUniqueName() const
{
    return String (getIndexInParent());
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

void SymbolDetailsView::SymbolItem::itemClicked (const MouseEvent& e)
{
    if (e.mods.isPopupMenu())
    {
        auto tree = dynamic_cast<SymbolDetailsView*> (getOwnerView());

        PopupMenu menu;
        menu.addItem (1, "Create Line Plot", tree->getNumSelectedItems() == 2);

        menu.showMenuAsync (PopupMenu::Options(), [tree, this] (int code)
        {
            switch (code)
            {
                case 1:
                    tree->listeners.call (&Listener::symbolDetailsWantsNewDefinition,
                                          "linePlot",
                                          tree->createExpressionFromSelectedSymbols ("line-plot"));
                    break;
                default: break;
            }
        });
    }
}

void SymbolDetailsView::SymbolItem::itemDoubleClicked (const MouseEvent& e)
{
    auto tree = dynamic_cast<SymbolDetailsView*> (getOwnerView());
    tree->listeners.call (&Listener::symbolDetailsItemPunched, getExpressionForPathInSymbol().toStdString());
}

void SymbolDetailsView::SymbolItem::itemSelectionChanged (bool isNowSelected)
{
}




//==============================================================================
SymbolDetailsView::SymbolDetailsView()
{
    setIndentSize (12);
    setMultiSelectEnabled (true);
    setRootItemVisible (true);
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

std::string SymbolDetailsView::createExpressionFromSelectedSymbols (const std::string &head) const
{
    return ("(" + getExpressionsForPathsInSelectedSymbols (head).joinIntoString (" ") + ")").toStdString();
}

StringArray SymbolDetailsView::getExpressionsForPathsInSelectedSymbols (const String& keyToPrepend) const
{
    StringArray expressions;

    for (int n = 0; n < getNumSelectedItems(); ++n)
    {
        auto item = dynamic_cast<SymbolItem*> (getSelectedItem(n));
        expressions.add (item->getExpressionForPathInSymbol());
    }

    if (keyToPrepend.isNotEmpty())
        expressions.insert (0, keyToPrepend);

    return expressions;
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

bool SymbolDetailsView::keyPressed (const KeyPress& key)
{
    if (key == KeyPress::spaceKey)
    {
        auto item = dynamic_cast<SymbolItem*> (getSelectedItem (0));
        listeners.call (&Listener::symbolDetailsItemPunched, item->getExpressionForPathInSymbol().toStdString());
        return true;
    }
    return TreeView::keyPressed (key);
}
