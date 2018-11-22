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
        virtual void symbolDetailsItemPunched (const std::string& expression) = 0;
        virtual void symbolDetailsWantsNewDefinition (const std::string& key, const std::string& expression) = 0;
    };

    //==========================================================================
    class SymbolItem : public TreeViewItem
    {
    public:
        SymbolItem (const std::string& key, const mcl::Object& object);
        String getExpressionForPathInSymbol() const;

        //==========================================================================
        String getUniqueName() const override;
        bool mightContainSubItems() override;
        void paintItem (Graphics& g, int width, int height) override;
        void itemClicked (const MouseEvent&) override;
        void itemDoubleClicked (const MouseEvent&) override;
        void itemSelectionChanged (bool isNowSelected) override;

        //==========================================================================
        std::string key;
        mcl::Object object;
    };

    //==============================================================================
    SymbolDetailsView();
    void addListener (Listener*);
    void removeListener (Listener*);
    void setViewedObject (const std::string& key, const mcl::Object& objectToView);
    const std::string& getCurrentSymbol() const;
    std::string createExpressionFromSelectedSymbols (const std::string& head) const;
    StringArray getExpressionsForPathsInSelectedSymbols (const String& keyToPrepend="") const;
    StringArray getSelectedKeys() const;

    //==============================================================================
    void focusGained (FocusChangeType) override;
    void focusLost (FocusChangeType) override;
    void focusOfChildComponentChanged (FocusChangeType) override;
    bool keyPressed (const KeyPress& key) override;

private:
    ListenerList<Listener> listeners;
    std::string currentKey;
    std::unique_ptr<SymbolItem> root;
    std::map<std::string, std::unique_ptr<XmlElement>> opennessStates;
};
