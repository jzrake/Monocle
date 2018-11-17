#include <set>
#include "Object.hpp"
#include "Expression.hpp"
#define BINARY_DATA_HEADER "mcl::Object"
#define BINARY_DATA_FORMAT "1.0.0"
using namespace mcl;




// ============================================================================
class Object::Serializer
{
public:
    std::vector<char> serialize (const Object& value)
    {
        packString (BINARY_DATA_HEADER);
        packString (BINARY_DATA_FORMAT);
        packObject (value);
        return std::move (bytes);
    }

    Object deserialize (const std::vector<char>& bytes)
    {
        iter = bytes.begin();
        end = bytes.end();

        auto header = unpackString();
        auto format = unpackString();

        if (header != BINARY_DATA_HEADER)
        {
            throw std::runtime_error ("mcl::Object deserialization got unfamiliar data format");
        }
        if (format != "1.0.0")
        {
            throw std::runtime_error ("mcl::Object deserialization got data with wrong version string");
        }
        return unpackObject();
    }

private:
    template<typename T>
    void pack (const T& value)
    {
        auto p = (char*)& value;
        bytes.insert (bytes.end(), p, p + sizeof (T));
    }

    template<typename T>
    T unpack()
    {
        if (iter + sizeof (T) > end)
        {
            throw std::runtime_error ("mcl::Object deserialization got corrupted data");
        }
        T value;
        std::memcpy (&value, &*iter, sizeof (T));
        iter += sizeof (T);
        return value;
    }

    void packList (const List& items)
    {
        pack (items.size());

        for (const auto& item : items)
        {
            packObject (item);
        }
    }

    List unpackList()
    {
        auto items = List();
        auto size = unpack<std::size_t>();

        for (std::size_t n = 0; n < size; ++n)
        {
            items.push_back (unpackObject());
        }
        return items;
    }

    void packDict (const Dict& items)
    {
        pack (items.size());

        for (const auto& item : items)
        {
            packString (item.first);
            packObject (item.second);
        }
    }

    Dict unpackDict()
    {
        auto items = Dict();
        auto size = unpack<std::size_t>();

        for (std::size_t n = 0; n < size; ++n)
        {
            auto key = unpackString();
            auto val = unpackObject();
            items.emplace (key, val);
        }
        return items;
    }

    void packExpr (const Expr& expr)
    {
        packString (expr.source);
    }

    Expr unpackExpr()
    {
        return unpackString();
    }

    void packData (const Data& data)
    {
        packString (data.v->serialize());
    }

    Data unpackData()
    {
        assert(false); // need to utilize custom de-serializer object, gets type from name
        return Data();
    }

    void packString (const std::string& str)
    {
        pack (str.size());

        for (const auto& c : str)
        {
            pack (c);
        }
    }

    std::string unpackString()
    {
        auto str = std::string();
        auto size = unpack<std::size_t>();

        for (std::size_t n = 0; n < size; ++n)
        {
            str += unpack<char>();
        }
        return str;
    }

    void packObject (const Object& value)
    {
        switch (value.v.index())
        {
            case 0: pack ('n'); break;
            case 1: pack ('b'); pack (value.get<bool>()); break;
            case 2: pack ('i'); pack (value.get<int>()); break;
            case 3: pack ('d'); pack (value.get<double>()); break;
            case 4: pack ('L'); packList (value.get<List>()); break;
            case 5: pack ('D'); packDict (value.get<Dict>()); break;
            case 6: pack ('U'); packData (value.get<Data>()); break;
            case 7: pack ('E'); packExpr (value.get<Expr>()); break;
            case 8: pack ('F'); packString ("<function>"); break;
            case 9: pack ('S'); packString (value.get<std::string>()); break;
        }
    }

    Object unpackObject()
    {
        switch (unpack<char>())
        {
            case 'n': return Object::None();
            case 'b': return unpack<bool>();
            case 'i': return unpack<int>();
            case 'd': return unpack<double>();
            case 'L': return unpackList();
            case 'D': return unpackDict();
            case 'U': return unpackData();
            case 'E': return unpackExpr();
            case 'F': return unpackString();
            case 'S': return unpackString();
        }
        throw;
    }

    std::vector<char> bytes;
    std::vector<char>::const_iterator iter;
    std::vector<char>::const_iterator end;
};




// ============================================================================
Object Object::deserialize (const std::vector<char>& data)
{
    return Serializer().deserialize (data);
}

char Object::type() const
{
    switch (v.index())
    {
        case 0: return 'n';
        case 1: return 'b';
        case 2: return 'i';
        case 3: return 'd';
        case 4: return 'L';
        case 5: return 'D';
        case 6: return 'U';
        case 7: return 'E';
        case 8: return 'F';
        case 9: return 'S';
    }
    return 0;
}

const Object& Object::operator[] (const char* index) const
{
    return mpark::get<Dict>(v).at (index);
}

const Object& Object::operator[] (const std::string& index) const
{
    return mpark::get<Dict>(v).at (index);
}

const Object& Object::operator[] (const std::size_t& index) const
{
    return mpark::get<List>(v).at (index);
}

Object& Object::operator[] (const char* index)
{
    return mpark::get<Dict>(v)[index];
}

Object& Object::operator[] (const std::string& index)
{
    return mpark::get<Dict>(v)[index];
}

Object& Object::operator[] (const std::size_t& index)
{
    return mpark::get<List>(v).at (index);
}

Object Object::with (const char* index, const Object& value) const
{
    auto object = *this;
    object[index] = value;
    return object;
}

Object Object::with (const std::string& index, const Object& value) const
{
    auto object = *this;
    object[index] = value;
    return object;
}

Object Object::with (const std::size_t& index, const Object& value) const
{
    auto object = *this;
    object[index] = value;
    return object;
}

Object Object::including (const Object& other) const
{
    auto object = *this;

    for (const auto& item : other.get<Dict>())
    {
        object[item.first] = item.second;
    }
    return object;
}

Object Object::pushing (const Object& other) const
{
    auto object = *this;
    object.get<Object::List>().push_back (other);
    return object;
}

Object Object::popping() const
{
    auto object = *this;

    if (object.get<Object::List>().size() > 0)
        object.get<Object::List>().pop_back();
    return object;
}

std::set<std::string> Object::symbols() const
{
    std::set<std::string> symb;

    switch (type())
    {
        case 'E': try { return Expression (get<Expr>().source).symbols(); } catch (...) { return {}; }
        case 'L': for (const auto& e : get<List>()) { auto s = e       .symbols(); symb.insert (s.begin(), s.end()); } return symb;
        case 'D': for (const auto& d : get<Dict>()) { auto s = d.second.symbols(); symb.insert (s.begin(), s.end()); } return symb;
        default: return {};
    }
}

Object Object::resolve (const Dict& scope) const
{
    switch (type())
    {
        case 'E': return Expression (get<Expr>().source).evaluate (scope);
        case 'L': { auto resolved = get<List>(); for (auto& e : resolved) e        = e       .resolve (scope); return resolved; }
        case 'D': { auto resolved = get<Dict>(); for (auto& d : resolved) d.second = d.second.resolve (scope); return resolved; }
        default: return *this;
    }
}

Object Object::resolve (Scope scope) const
{
    switch (type())
    {
        case 'E': return Expression (get<Expr>().source).evaluate (scope);
        case 'L': { auto resolved = get<List>(); for (auto& e : resolved) e        = e       .resolve (scope); return resolved; }
        case 'D': { auto resolved = get<Dict>(); for (auto& d : resolved) d.second = d.second.resolve (scope); return resolved; }
        default: return *this;
    }
}

std::string Object::protocol() const
{
    if (type() == 'D')
        if (get<Dict>().count ("__protocol__"))
            if (get<Dict>().at ("__protocol__").type() == 'S')
                return get<Dict>().at ("__protocol__").get<std::string>();

    return std::string();
}

std::string Object::expression() const
{
    return type() == 'E' ? get<Expr>().source : std::string();
}

std::vector<char> Object::serialize() const
{
    return Serializer().serialize (*this);
}




// ============================================================================
#include <cassert>
#include "Builtin.hpp"

void Object::testSerialization()
{
    // Test that deserialization is the inverse of serialization
    {
        auto testObject = [] (const Object& value)
        {
            assert (Object::deserialize (value.serialize()) == value);
        };

        testObject (666);
        testObject (12.123);
        testObject (std::string (""));
        testObject (std::string ("some content"));
        testObject (Object::dict());
        testObject (Object::dict().with ("key1", 123).with ("key2", Object::list()));
        testObject (Object::list().pushing (123).pushing (456));
    }

    // Test that data deserialization does not overflow in the event of corrupted data
    {
        auto caught = false;
        auto bytes = Object::list().serialize();
        bytes.pop_back();

        try
        {
            Object::deserialize (bytes);
        }
        catch (std::runtime_error)
        {
            caught = true;
        }
        assert (caught);
    }
}

void Object::testSymbolResolution()
{
    auto scope = Object::dict()
    .with ("a", 1.0)
    .with ("b", 2.0)
    .including (Builtin::arithmetic())
    .get<Dict>();

    assert (Object::expr ("(add a b)").resolve (scope) == 3.0);
    assert (Object::dict()
            .with ("A", Object::Expr ("(add a b)"))
            .with ("B", Object::Expr ("(sub a b)")).resolve (scope)["A"] == 3.0);
    assert (Object::dict()
            .with ("A", Object::Expr ("(add a b)"))
            .with ("B", Object::Expr ("(sub a b)")).resolve (scope)["B"] ==-1.0);
}
