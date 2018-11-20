#include "SymbolListView.hpp"
#include "MaterialIcons.hpp"




//==============================================================================
SymbolListView::SymbolListView()
{
    setModel (this);
    setOutlineThickness (0);
    setMultipleSelectionEnabled (true);
    setColour (ListBox::ColourIds::outlineColourId, Colours::transparentBlack);

    iconNone       = material::util::icon (material::toggle::ic_radio_button_unchecked, Colours::grey);
    iconNumeric    = material::util::icon (material::image::ic_exposure_zero, Colours::black);
    iconString     = material::util::icon (material::content::ic_text_format, Colours::darkred);
    iconExpression = material::util::icon (material::image::ic_edit, Colours::darkorchid);
    iconFunction   = material::util::icon (material::editor::ic_functions, Colours::purple);
    iconDict       = material::util::icon (material::av::ic_library_books, Colours::purple);
    iconList       = material::util::icon (material::editor::ic_format_list_numbered, Colours::darkmagenta);
    iconAny        = material::util::icon (material::device::ic_storage, Colours::purple);
    iconRightArrow = material::util::icon (material::navigation::ic_arrow_forward, Colours::black);
    iconError      = material::util::icon (material::alert::ic_error, Colours::red);
}

void SymbolListView::addListener (Listener* listener)
{
    listeners.add (listener);
}

void SymbolListView::removeListener (Listener* listener)
{
    listeners.remove (listener);
}

void SymbolListView::setSymbolList (const std::vector<Status> &statusesToDisplay)
{
    statuses = statusesToDisplay;
    updateContent();

    if (getSelectedRow() == -1)
        selectRow (int (statuses.size()) - 1);
}

StringArray SymbolListView::getSelectedSymbols() const
{
    StringArray selectedSymbols;

    for (int n = 0; n < statuses.size(); ++n)
        if (isRowSelected (n))
            selectedSymbols.add (statuses[n].at ("key"));
    return selectedSymbols;
}

void SymbolListView::updateSymbolStatus (const std::string& key, const Status& status)
{
    int row = findSymbol (key);

    if (status.at ("exist").empty()) // symbol is not in the kernel
    {
        if (row != -1) // symbol is in the list box
        {
            statuses.erase (statuses.begin() + row);
            updateContent();
        }
    }
    else // symbol is in the kernel
    {
        if (row != -1) // symbol is in the list box
        {
            statuses[row] = status;
            repaintRow (row);
        }
        else // symbol is not in the list box
        {
            statuses.push_back (status);
            auto s = [] (const auto& a, const auto& b) { return a.at ("key") < b.at ("key"); };
            std::sort (statuses.begin(), statuses.end(), s);
            updateContent();
        }
    }
    listeners.call (&Listener::symbolListSelectionChanged, getSelectedSymbols());
}

//==============================================================================
void SymbolListView::focusGained (FocusChangeType)
{
    repaint();
}

void SymbolListView::focusLost (FocusChangeType)
{
    repaint();
}

bool SymbolListView::keyPressed (const KeyPress& key)
{
    if (getNumSelectedRows() == 1 && key == KeyPress::spaceKey)
    {
        listeners.call (&Listener::symbolListSymbolPunched, getSelectedSymbols()[0]);
        return true;
    }
    return ListBox::keyPressed (key);
}

//==============================================================================
int SymbolListView::getNumRows()
{
    return int (statuses.size());
}

void SymbolListView::paintListBoxItem (int row, Graphics& g, int w, int h, bool selected)
{
    if (selected)
    {
        g.setColour (hasKeyboardFocus (false) ? Colours::lightblue : Colours::lightgrey);
        g.fillRect (0, 0, w, h);
    }

    auto key     =   statuses[row].at ("key");
    auto type    =   statuses[row].at ("type");
    auto expr    =   statuses[row].at ("expr");
    auto isExpr  = ! statuses[row].at ("expr").empty();
    auto isError = ! statuses[row].at ("error").empty();
    auto extra   = std::string();

    auto iconAreaL = Rectangle<float> (0, 0, h, h);
    auto iconAreaC = Rectangle<float> (w - 2 * h, 0, h, h);
    auto iconAreaR = Rectangle<float> (w - 1 * h, 0, h, h);
    auto iconRectL = iconAreaL.withSizeKeepingCentre (10, 10);
    auto iconRectC = iconAreaC.withSizeKeepingCentre (10, 10);
    auto iconRectR = iconAreaR.withSizeKeepingCentre (10, 10);

    Drawable* iconL = isExpr ? iconExpression.get() : getIconForType (statuses[row].at ("type").front());
    Drawable* iconC = isExpr ? iconRightArrow.get() : nullptr;
    Drawable* iconR = isExpr ? getIconForType (statuses[row].at ("type").front()) : nullptr;

    if (isError)
        iconR = iconError.get();
    if (isExpr)
        extra = " = " + expr;

    g.setColour (Colours::black);
    g.setFont (Font ("Monaco", 12, 0));
    g.drawText (key + extra, h, 0, w - 3 * h, h, Justification::centredLeft);

    if (iconL) iconL->drawWithin (g, iconRectL, RectanglePlacement::fillDestination, 1.f);
    if (iconC) iconC->drawWithin (g, iconRectC, RectanglePlacement::fillDestination, 1.f);
    if (iconR) iconR->drawWithin (g, iconRectR, RectanglePlacement::fillDestination, 1.f);
}

void SymbolListView::listBoxItemClicked (int row, const MouseEvent& e)
{
    if (e.mods.isPopupMenu())
    {
        PopupMenu menu;
        menu.addItem (1, String ("Remove symbol") + (getNumSelectedRows() > 1 ? "s" : ""));
        menu.showMenuAsync(PopupMenu::Options(), [this] (int code)
        {
            switch (code)
            {
                case 1: sendDeleteSelectedSymbols(); break;
                default: break;
            }
        });
    }
}

void SymbolListView::listBoxItemDoubleClicked (int row, const MouseEvent&)
{
}

void SymbolListView::backgroundClicked (const MouseEvent&)
{
    deselectAllRows();
}

void SymbolListView::selectedRowsChanged (int)
{
    listeners.call (&Listener::symbolListSelectionChanged, getSelectedSymbols());
}

void SymbolListView::deleteKeyPressed (int)
{
    sendDeleteSelectedSymbols();
}

void SymbolListView::returnKeyPressed (int)
{
}

void SymbolListView::listWasScrolled()
{
}

String SymbolListView::getTooltipForRow (int row)
{
    return statuses[row].at ("error") + statuses[row].at ("doc");
}

//==============================================================================
void SymbolListView::sendDeleteSelectedSymbols()
{
    auto newSelection = getSelectedRows().getRange(0).getStart();
    auto filesToDelete = getSelectedSymbols();
    listeners.call (&Listener::symbolListSymbolsRemoved, getSelectedSymbols());
    selectRow (newSelection);
}

int SymbolListView::findSymbol (const std::string& key)
{
    int n = 0;

    for (const auto& status : statuses)
    {
        if (status.at ("key") == key)
            break;
        ++n;
    }
    return n < statuses.size() ? n : -1;
    
}

Drawable* SymbolListView::getIconForType (char type)
{
    switch (type)
    {
        case 'b': case 'i': case 'd': return iconNumeric.get();
        case 'n': return iconNone.get();
        case 'A': return iconAny.get();
        case 'L': return iconList.get();
        case 'D': return iconDict.get();
        case 'S': return iconString.get();
        case 'E': return iconExpression.get();
        case 'F': return iconFunction.get();
        default: return nullptr;
    }
}
