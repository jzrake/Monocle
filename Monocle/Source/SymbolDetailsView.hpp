#pragma once
#include "JuceHeader.h"
#include "Kernel/Object.hpp"




//==============================================================================
class SymbolDetailsView : public TreeView
{
public:

    //==========================================================================
    class Listener
    {
    public:
        virtual ~Listener() {}
        virtual void symbolDetailsWantsNewDefinition (int code, const StringArray& argumentKeys) = 0;
    };

    //==========================================================================
    class SymbolItem : public TreeViewItem
    {
    public:
        SymbolItem (const std::string& key, const mcl::Object& object);
        bool mightContainSubItems() override;
        void paintItem (Graphics& g, int width, int height) override;
        String getUniqueName() const override;

        //==========================================================================
        void itemClicked (const MouseEvent&) override;
        void itemSelectionChanged (bool isNowSelected) override;

        //==========================================================================
        std::string key;
        mcl::Object object;
    };

    SymbolDetailsView();
    void addListener (Listener*);
    void removeListener (Listener*);
    void setViewedObject (const std::string& key, const mcl::Object& objectToView);
    const std::string& getCurrentSymbol() const;
    StringArray getSelectedKeys() const;

    //==============================================================================
    void focusGained (FocusChangeType) override;
    void focusLost (FocusChangeType) override;
    void focusOfChildComponentChanged (FocusChangeType) override;

private:
    ListenerList<Listener> listeners;
    std::string currentKey;
    std::unique_ptr<SymbolItem> root;
    std::map<std::string, std::unique_ptr<XmlElement>> opennessStates;
};
