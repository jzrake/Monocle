#include "Database.hpp"




// ============================================================================
Database::Database()
{
}

void Database::addListener (Listener* listener)
{
    listeners.add (listener);
}

void Database::removeListener (Listener* listener)
{
    listeners.remove (listener);
}

void Database::addEntry (const Entry& entry)
{
    entries.emplace (entry.symbol, entry);
}

void Database::removeEntry (const Identifier& symbol)
{
    entries.erase (symbol);
}

Array<Database::Entry> Database::getEntries() const
{
    Array<Entry> e;
    
    for (const auto& pair : entries)
        e.add (pair.second);
    return e;
}
