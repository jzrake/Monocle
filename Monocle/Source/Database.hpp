#pragma once
#include <map>
#include "JuceHeader.h"




// ============================================================================
class Database
{
public:
    // ========================================================================
    struct Entry
    {
        Identifier symbol;
        std::shared_ptr<std::vector<double>> data;
        std::vector<size_t> shape;
    };

    // ========================================================================
    class Listener
    {
    public:
        virtual ~Listener() {}
    };

    // ========================================================================
    Database();
    void addListener (Listener* listener);
    void removeListener (Listener* listener);
    void addEntry (const Entry& entry);
    void removeEntry (const Identifier& symbol);
    Array<Entry> getEntries() const;

private:
    std::map<Identifier, Entry> entries;
    ListenerList<Listener> listeners;
};
