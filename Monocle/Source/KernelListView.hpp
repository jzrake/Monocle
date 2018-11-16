#pragma once
#include <unordered_map>
#include "JuceHeader.h"



//==============================================================================
class KernelListView
: public ListBox
, public ListBoxModel
{
public:
    using Status = std::unordered_map<std::string, std::string>;

    //==========================================================================
    class Listener
    {
    public:
        virtual ~Listener() {}
        virtual void kernelListSelectionChanged (const StringArray& symbols) = 0;
        virtual void kernelListSymbolsRemoved (const StringArray& symbols) = 0;
    };

    //==========================================================================
    KernelListView();
    void addListener (Listener* listener);
    void removeListener (Listener* listener);
    void setSymbolList (const std::vector<Status>& statusesToDisplay);
    StringArray getSelectedSymbols() const;
    void updateSymbolStatus (const std::string& key, const Status& status);

    //==========================================================================
    void focusGained (FocusChangeType) override;
    void focusLost (FocusChangeType) override;

    //==========================================================================
    int getNumRows() override;
    void paintListBoxItem (int, Graphics&, int, int, bool) override;
    void listBoxItemClicked (int, const MouseEvent&) override;
    void listBoxItemDoubleClicked (int, const MouseEvent&) override;
    void backgroundClicked (const MouseEvent&) override;
    void selectedRowsChanged (int) override;
    void deleteKeyPressed (int) override;
    void returnKeyPressed (int) override;
    void listWasScrolled() override;
    String getTooltipForRow (int) override;

private:
    void sendDeleteSelectedSymbols();
    Drawable* getIconForType (char type);

    std::vector<Status> statuses;
    ListenerList<Listener> listeners;
    std::unique_ptr<Drawable> iconNone;
    std::unique_ptr<Drawable> iconString;
    std::unique_ptr<Drawable> iconNumeric;
    std::unique_ptr<Drawable> iconAny;
    std::unique_ptr<Drawable> iconExpression;
    std::unique_ptr<Drawable> iconList;
    std::unique_ptr<Drawable> iconDict;
    std::unique_ptr<Drawable> iconFunction;
    std::unique_ptr<Drawable> iconRightArrow;
};
