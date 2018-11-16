#include "AcyclicGraph.hpp"
using namespace mcl;




// ============================================================================
void AcyclicGraph::setListener (Listener listenerToInvoke)
{
    listener = listenerToInvoke;
}

void AcyclicGraph::setErrorLog (ErrorLog errorLogToInvoke)
{
    errorLog = errorLogToInvoke;
}

bool AcyclicGraph::insert (const std::string& key, const Object& value, const std::set<std::string>& incoming)
{
    if (wouldCreateCycle (key, incoming))
        return false;

    /*
     getOutgoingEdges is O(N) if it's not already in the graph.
     */
    auto outgoing = getOutgoingEdges (key);

    if (contains (key))
    {
        auto cachedListener = listener;
        listener = nullptr; /* prevents listener message from going out twice */

        remove (key);
        listener = cachedListener;
    }

    /*
     Add the new node as an outoing edge for all of its incomings.
     */
    for (const auto& i : incoming)
        if (contains (i))
            nodes.at (i).outgoing.insert (key);

    Node node;
    node.key = key;
    node.abstract = value;
    node.concrete = resolve (value, node.error);
    node.incoming = incoming;
    node.outgoing = outgoing;

    if (listener)
        listener (key, node.concrete);

    for (const auto& o : node.outgoing)
        mark (o);

    nodes.emplace (key, node);
    updateRecurse (key);

    return true;
}

bool AcyclicGraph::insert (const std::string& key, const Object& item)
{
    return insert (key, item, item.symbols());
}

std::string AcyclicGraph::insert (const Object& item)
{
    auto key = nextRandomKey();
    insert (key, item);
    return key;
}

std::string AcyclicGraph::nextRandomKey() const
{
    static const char allowedCharacters[27] = "abcdefghijklmnopqrstuvwxyz";

    std::string key;
    key.resize (4);

    /*
     Note: 26**4 is roughly 0.5 million. If scaling to large graphs this number
     could get slow or even cause an infinite loop.
     */
    do {
        std::generate (key.begin(), key.end(), [] () { return allowedCharacters[rand() % 26]; });
    } while (contains (key));

    return key;
}

std::string AcyclicGraph::nextUniqueKey (const std::string &prefix) const
{
    std::string key;
    int n = 0;

    do {
        key = prefix + std::to_string (++n);
    } while (contains (key));

    return key;
}

void AcyclicGraph::import (const Object::Dict &items)
{
    for (const auto& item : items)
        insert (item.first, item.second, item.second.symbols());
}

bool AcyclicGraph::remove (const std::string& key)
{
    auto node = nodes.find (key);

    if (node == nodes.end())
        return false;

    for (const auto& o : node->second.outgoing)
        mark (o);

    for (const auto& i : node->second.incoming)
    {
        auto upstream = nodes.find (i);

        if (upstream != nodes.end())
            upstream->second.outgoing.erase (key);
    }

    if (listener && ! node->second.concrete.empty())
        listener (key, Object());

    nodes.erase (node);
    dirty.erase (key);

    return true;
}

void AcyclicGraph::clear()
{
    nodes.clear();
    dirty.clear();
}

std::size_t AcyclicGraph::size() const
{
    return nodes.size();
}

bool AcyclicGraph::contains (const std::string& key) const
{
    return nodes.find (key) != nodes.end();
}

const Object& AcyclicGraph::abstract (const std::string& key) const
{
    static Object empty;
    auto node = nodes.find (key);

    if (node == nodes.end())
        return empty;

    return node->second.abstract;
}

const Object& AcyclicGraph::concrete (const std::string& key) const
{
    static Object empty;
    auto node = nodes.find (key);

    if (node == nodes.end())
        return empty;

    return node->second.concrete;
}

const std::string& AcyclicGraph::error (const std::string& key) const
{
    static std::string empty;

    if (! contains (key))
        return empty;

    return nodes.at (key).error;
}

Object::List AcyclicGraph::select (NodePredicate predicate) const
{
    auto s = Object::List();

    for (const auto& node : nodes)
        if (predicate (node.second))
            s.push_back (node.first);

    std::sort (s.begin(), s.end(), [] (const Object& a, const Object& b)
    {
        return a.get<std::string>() < b.get<std::string>();
    });
    return s;
}

Object::Dict AcyclicGraph::scope() const
{
    auto s = Object::Dict();

    for (const auto& node : nodes)
        s[node.first] = node.second.concrete;

    return s;
}

Object::Dict AcyclicGraph::scope (const std::set<std::string>& keys) const
{
    auto s = Object::Dict();

    for (const auto& key : keys)
        s[key] = concrete (key);

    return s;
}

Object AcyclicGraph::resolve (const Object& object, std::string& error) const
{
    try {
        error.clear();
        return resolve (object);

    }
    catch (std::exception& e)
    {
        error = e.what();
        return Object::None();
    }
}

Object AcyclicGraph::resolve (const Object& object) const
{
    return object.resolve ([this] (const std::string& key) { return concrete (key); });
}

bool AcyclicGraph::update (const std::string& key)
{
    auto node = nodes.find (key);

    if (node == nodes.end())
        return true;

    if (current (key))
        return true;

    if (! current (node->second.incoming))
        return false;

    node->second.concrete = resolve (node->second.abstract, node->second.error);
    dirty.erase (key);

    if (listener)
        listener (key, node->second.concrete);

    if (errorLog && ! node->second.error.empty())
        errorLog (key, node->second.error);

    return true;
}

void AcyclicGraph::updateRecurse (const std::string& key)
{
    if (update (key))
        for (const auto& o : getOutgoingEdges (key))
            updateRecurse (o);
}

bool AcyclicGraph::current (const std::string& key) const
{
    return dirty.count (key) == 0;
}

bool AcyclicGraph::current (const std::set<std::string>& keys) const
{
    for (const auto& key : keys)
        if (! current (key))
            return false;

    return true;
}

std::set<std::string> AcyclicGraph::getIncomingEdges (const std::string& key) const
{
    static std::set<std::string> empty;
    auto node = nodes.find (key);

    if (node == nodes.end())
        return empty;

    return node->second.incoming;
}

std::set<std::string> AcyclicGraph::getOutgoingEdges (const std::string& key) const
{
    auto node = nodes.find (key);

    if (node == nodes.end())
    {
        std::set<std::string> outgoing;
        /*
         Search the graph for nodes naming key as a dependency, and add those nodes
         to the list of outgoing edges.
         */
        for (const auto& other : nodes)
        {
            const auto& i = other.second.incoming;

            if (std::find (i.begin(), i.end(), key) != i.end())
                outgoing.insert (other.first);
        }
        return outgoing;
    }
    return node->second.outgoing;
}

bool AcyclicGraph::isDownstreamOf (const std::string& source, const std::string& target) const
{
    if (source == target)
        return true;

    for (const auto& downstreamOfTarget : getOutgoingEdges (target))
        if (isDownstreamOf (source, downstreamOfTarget))
            return true;

    return false;
}

bool AcyclicGraph::isUpstreamOf (const std::string& source, const std::string& target) const
{
    if (source == target)
        return true;

    for (const auto& upstreamOfTarget : getIncomingEdges (target))
        if (isUpstreamOf (source, upstreamOfTarget))
            return true;

    return false;
}

bool AcyclicGraph::wouldCreateCycle (const std::string& source, const std::set<std::string>& incoming) const
{
    for (const auto& i : incoming)
        if (isUpstreamOf (source, i))
            return true;

    return false;
}

void AcyclicGraph::mark (const std::string& key)
{
    auto node = nodes.find (key);

    if (node != nodes.end())
    {
        if (! current (key))
            return;

        if (! node->second.incoming.empty())
            dirty.insert (key);

        for (const auto& o : node->second.outgoing)
            mark (o);
    }
}




// ============================================================================
#include <cassert>
#include "Expression.hpp"
#include "Builtin.hpp"

void AcyclicGraph::testTopologies()
{
    AcyclicGraph graph;

    assert (graph.insert ("a", Object(), {}));
    assert (graph.insert ("b", Object(), {}));
    assert (graph.insert ("c", Object(), {}));

    assert (graph.remove ("a"));
    assert (graph.remove ("b"));
    assert (graph.remove ("c"));
    assert (graph.remove ("c") == false);
    assert (graph.size() == 0);

    assert (graph.insert ("a", Object(), {}));
    assert (graph.insert ("b", Object(), {}));
    assert (graph.insert ("c", Object(), {}));

    assert (graph.size() == 3);
    assert (graph.contains ("a"));
    assert (graph.isDownstreamOf ("c", "a") == false);
    assert (graph.isDownstreamOf ("c", "b") == false);
    assert (graph.insert ("c", Object(), {"a", "b"}));

    assert (graph.size() == 3);
    assert (graph.contains ("a"));
    assert (graph.isDownstreamOf ("c", "a") == true);
    assert (graph.isDownstreamOf ("c", "b") == true);

    assert (graph.wouldCreateCycle ("d", {}) == false);
    assert (graph.wouldCreateCycle ("d", {"d"}));
    assert (graph.insert ("d", Object(), {"d"}) == false);

    assert (graph.insert ("d", Object(), {"c"}));
    assert (graph.wouldCreateCycle ("b", {"d"}));
    assert (graph.size() == 4);
    assert (graph.contains ("d"));
    assert (graph.isUpstreamOf ("c", "d"));
    assert (graph.isDownstreamOf ("d", "c"));
    assert (graph.remove ("d") == true);
    assert (graph.isUpstreamOf ("c", "d") == false);
    assert (graph.isDownstreamOf ("d", "c") == false);
    assert (graph.wouldCreateCycle ("b", {"d"}) == false);

    assert (graph.remove ("b"));
    assert (graph.size() == 2);
    assert (graph.contains ("b") == false);
    assert (graph.isDownstreamOf ("c", "a") == true);
    assert (graph.isDownstreamOf ("c", "b") == true);

    graph.import (Builtin::arithmetic());
    auto expr = Object::expr ("(add 1 2)");
    assert (graph.contains ("add"));
    assert (expr.resolve (graph.scope (expr.symbols())) == 3);
    assert (graph.resolve (expr) == 3);

    assert (graph.insert("x", Object::expr ("2.0")));
    assert (graph.concrete ("x").type() == 'd');
}
