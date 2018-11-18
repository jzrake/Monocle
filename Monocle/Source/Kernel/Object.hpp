#pragma once
#include <string>
#include <vector>
#include <map>
#include <set>
#include "Variant.hpp"
#include "UserData.hpp"

namespace mcl { class Object; }




// ============================================================================
class mcl::Object
{
public:
    using Scope = std::function<Object(const std::string&)>;
    using List = std::vector<Object>;
    using Dict = std::map<std::string, Object>;

    struct None
    {
        bool operator==(const None& other) const { return true; }
        bool operator!=(const None& other) const { return false; }
    };

    struct Expr
    {
        Expr() {}
        Expr (const std::string& source) : source (source) {}
        bool operator==(const Expr& other) const { return source == other.source; }
        bool operator!=(const Expr& other) const { return source != other.source; }
        std::string source;
    };

    struct Func
    {
        Func() {}
        Func (std::function<Object (const List&, const Dict&)> f, const std::string& doc="") : f (f), doc (doc) {}
        bool operator==(const Func& other) const { return false; }
        bool operator!=(const Func& other) const { return true; }
        std::function<Object (const List&, const Dict&)> f = nullptr;
        std::string doc;
    };

    struct Data
    {
        Data() {}
        Data (std::shared_ptr<UserData> v) : v (v) {}
        bool operator==(const Data& other) const { return false; }
        bool operator!=(const Data& other) const { return true; }
        std::string describe() const { return v->describe(); }
        std::shared_ptr<UserData> v;
    };

    static Object none() { return None(); }
    static Object dict() { return Dict(); }
    static Object list() { return List(); }
    static Object data (std::shared_ptr<UserData> data) { return Data (data); }
    static Object expr (const std::string& expr) { return Expr (expr); }
    static Object deserialize (const std::vector<char>&);

    bool empty() const { return v.index() == 0; }
    char type() const;

    Object() : v (None()) {}
    template<typename T> Object (const T& v) : v (v) {}
    template <typename T> T& get() { return mpark::get<T>(v);}
    template <typename T> const T& get() const { return mpark::get<T>(v);}
    template <typename T> void set (const T& w) { v.emplace<T>(w); }
    template <typename T> Object& operator= (const T& w) { v.emplace<T>(w); return *this; }

    bool operator== (const Object& other) const { return v == other.v; }
    bool operator!= (const Object& other) const { return v != other.v; }
    const Object& operator[] (const char* index) const;
    const Object& operator[] (const std::string& index) const;
    const Object& operator[] (const std::size_t& index) const;

    Object& operator[] (const char* index);
    Object& operator[] (const std::string& index);
    Object& operator[] (const std::size_t& index);

    Object with (const char* index, const Object& value) const;
    Object with (const std::string& index, const Object& value) const;
    Object with (const std::size_t& index, const Object& value) const;

    /** Operations applying only to dictionary objects. */
    Object including (const Object& other) const;
    Object operator+ (const Object& other) const { return including (other); }

    /** Operations applying only to list objects. */
    Object pushing (const Object& other) const;
    Object popping() const;
    std::size_t size() const { return get<List>().size(); }
    const Object& front() const { return get<List>().front(); }
    const Object& back() const { return get<List>().back(); }
    const Object& index (std::size_t n) const { return get<List>().at (n); }

    /** Return a collection of symbol names appearing in any data members
        that are expressions.
     */
    std::set<std::string> symbols() const;

    /** Return a version of this object with all of its expression data
        members replaced with the result of their evaluation within
        the given scope. If any expressions contain unresolved symbols,
        this function throws an exception.
     */
    Object resolve (const Dict& scope) const;
    Object resolve (Scope scope) const;

    /** If this object is a dict, and has a string-valued attribute named __protocol__,
        this method will return that string. Otherwise it returs an empty string.
     */
    std::string protocol() const;

    /** If this object is an expression, return the expression string. Otherwise return
        empty.
     */
    std::string expression() const;

    /** Return a binary sequence represention of the object. Func and Any are not
        serialized.
     */
    std::vector<char> serialize() const;

    static void testSerialization();
    static void testSymbolResolution();

private:
    class Serializer;
    mpark::variant<
    None,
    bool,
    int,
    double,
    List,
    Dict,
    Data,
    Expr,
    Func,
    std::string> v;
};
