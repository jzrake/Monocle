#include <string>
#include <unordered_map>
#include <unordered_set>
#include "Object.hpp"

namespace mcl { class AcyclicGraph; }




// ============================================================================
/**
A directed acyclic graph data structure.

This class implements a directed acyclic graph. Nodes are identified by a
unique string key, and contain an Object data member. Edges in the graph
express the direction of data flow; if B has an "incoming edge", A -> B, then
B depends on A, or data flows from A to B. A node's incoming edges are unique
but not ordered. Edges must terminate on a concrete node, but they may
originate from nodes that are not actually in the graph. The data value of
nodes not in the graph is Object::None. Nodes have a dirty/clean status that
tracks the the insertion and removal of their upstream nodes.
*/
class mcl::AcyclicGraph
{
public:

    struct Node
    {
        Object abstract;
        Object concrete;
        std::string key;
        std::string error;
        std::set<std::string> incoming;
        std::set<std::string> outgoing;
    };
    using Status = std::unordered_map<std::string, std::string>;
    using NodePredicate = std::function<bool (const Node&)>;
    using NodeMap = std::unordered_map<std::string, Node>;
    using NodeSet = std::unordered_set<std::string>;

    /** Function type for notifications of concrete data updates.
    */
    using Listener = std::function<void (const std::string&, const Object&)>;

    /** Function type for notifications of failed downstream symbol resolutions.
     */
    using ErrorLog = std::function<void (const std::string& key, const std::string& what)>;

    /** Set a callback to be invoked with the key and concrete data of a node,
        any time that data is updated. The callback is invoked when a node is
        inserted only if that node has concrete data, and when it is removed
        unless the pre-existing value was already Object::None.
    */
    void setListener (Listener listenerToInvoke);

    /** Set a callback to be invoked when a downstream symbol resolution fails for any
        reason.
     */
    void setErrorLog (ErrorLog errorLogToInvoke);

    /** Insert the given node into the graph. If the node cannot be inserted because
        it would create a cycle, then returns false. Otherwise returns true. Incoming
        edges are inferred by calling item.symbols().
    */
    bool insert (const std::string& key, const Object& item);

    /** Insert the given node into the graph with an automatically generated key that
        does not already exist, and return that key. If the node would create a cycle
        then an empty string is returned.
     */
    std::string insert (const Object& item);

    /** Trigger an update of any expression downstream of the given symbol, even if the
        data associated with it has not changed.
     */
    void touch (const std::string& key);

    /** Generate a random key that does not exist in the graph. */
    std::string nextRandomKey() const;

    /** Generate a unique key by appending the an integer to the given prefix string. */
    std::string nextUniqueKey (const std::string& prefix) const;

    /** Insert a dictionary of object items into the graph. For each item that has
        dependent symbols, those symbols are registered as incoming edges.
     */
    void import (const Object::Dict& items);

    /** Remove a node from the graph. If the node does not exist then no action is
        taken and false is returned.
    */
    bool remove (const std::string& key);

    /** Clear the whole graph. */
    void clear();

    /** Return the number of nodes in the graph. */
    std::size_t size() const;

    /** Determine whether the given key exists in the graph. */
    bool contains (const std::string& key) const;

    /** Return the abstract data associated with the given key. Object::None is returned
        if the key does not exist.
    */
    const Object& abstract (const std::string& key) const;

    /** Return the concrete data associated with the given key. Object::None is returned
        if the key does not exist.
     */
    const Object& concrete (const std::string& key) const;

    /** Return the error string associated with the evaluation of a node. */
    const std::string& error (const std::string& key) const;

    /** Return a list of all key names with nodes satisfying the given predicate. If
        the predicate is nullptr, then all the symbol names are returned.
     */
    std::vector<std::string> select (NodePredicate predicate=nullptr) const;

    /** Return a dictionary describing the symbols:

        key ..... the symbol's key name
        type .... a single character corresponding to the concrete object type
        expr .... the expression string if the abstract object is an expression
        descr ... a succinct string description of the concrete object (not yet implemented)
        error ... a non-empty string if it's an expression and its evaluation has failed
     */
    Status status (const std::string& key) const;

    std::vector<Status> status (const std::vector<std::string>& keys) const;

    /** Return a mapping of all the data items in the graph to their concrete
        values.
     */
    Object::Dict scope() const;

    /** Return a mapping of the concrete data items associated with the given keys.
     */
    Object::Dict scope (const std::set<std::string>& keys) const;

    /** Call the resolve method on the given object using the graph concrete data as
        the scope. If any exceptions are thrown, they are caught and the what() value
        is returned in the error string.
     */
    Object resolve (const Object& object, std::string& error) const;

    /** Same as above, but exceptions are not caught. */
    Object resolve (const Object& object) const;

    /** Update the concrete data of the given node, if necessary and possible.
        The update is only necessary if the node is dirty, and is not possible
        if the node has any dirty upstream nodes. Returns true if the node
        is current after the call. Invokes the listener on the node if it was
        updated.
    */
    bool update (const std::string& key);

    /** Update the concrete data of the given node, if necessary and possible.
        Then recurse to all of the node's outgoing edges and update them if
        possible. The listener is invoked once for each node whose concrete
        data is updated.
     */
    void updateRecurse (const std::string& key);

    /** Determine whether the concrete value of the given node has been updated
        since any of its upstream nodes have changed. Concrete nodes and nodes
        that do not exist are current. Current is the opposite of dirty.
    */
    bool current (const std::string& key) const;

    /** Convenience function to check whether all the given keys are current. */
    bool current (const std::set<std::string>& keys) const;

    /** Return the incoming edges for the given node. An empty list is returned if
        the key does not exist.
    */
    std::set<std::string> getIncomingEdges (const std::string& key) const;

    /** Return the outgoing edges for the given node. Even if that node does not exist
        in the graph, it may have outgoing edges if other nodes in the graph name it
        as a dependency.
    */
    std::set<std::string> getOutgoingEdges (const std::string& key) const;

    /** Determine whether the given source node is downstream of, or identical to,
        the target node.
    */
    bool isDownstreamOf (const std::string& source, const std::string& target) const;

    /** Determine whether the given source node is upstream of, or identical to,
        the target node.
    */
    bool isUpstreamOf (const std::string& source, const std::string& target) const;

    /** Determine whether the addition of a node called source, with the given
        incoming edges, would create a cycle in the graph. This simply checks
        whether any of the proposed dependencies depend on a node named source.
    */
    bool wouldCreateCycle (const std::string& source, const std::set<std::string>& incoming) const;

    static void testTopologies();

private:
    bool insert (const std::string& key, const Object& value, const std::set<std::string>& incoming);
    bool removeWithoutNotificationOrUpdate (const std::string& key);
    void mark (const std::string& key);

    NodeMap nodes;
    NodeSet dirty;
    Listener listener = nullptr;
    ErrorLog errorLog = nullptr;
    
};
