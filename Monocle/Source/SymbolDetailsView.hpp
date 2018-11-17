#pragma once
#include "JuceHeader.h"
#include "Kernel/Object.hpp"




//==============================================================================
class SymbolDetailsView : public TreeView
{
public:
    class SymbolItem : public TreeViewItem
    {
    public:
        SymbolItem (const std::string& key, const mcl::Object& object);
        bool mightContainSubItems() override;
        void paintItem (Graphics& g, int width, int height) override;
        String getUniqueName() const override;

        std::string key;
        mcl::Object object;
    };

    SymbolDetailsView();
    void setViewedObject (const std::string& key, const mcl::Object& objectToView);
    const std::string& getCurrentSymbol() const;

    void focusGained (FocusChangeType) override;
    void focusLost (FocusChangeType) override;
    void focusOfChildComponentChanged (FocusChangeType) override;

private:
    std::string currentKey;
    std::unique_ptr<SymbolItem> root;
    std::map<std::string, std::unique_ptr<XmlElement>> opennessStates;
};
