#pragma once
#include "JuceHeader.h"
#include "3rdParty/crt-kernel/kernel.hpp"
#include "3rdParty/ndarray/ndarray.hpp"




// ============================================================================
class VarCallAdapter
{
public:
    using ObjectType = var;
    using list_t = std::vector<ObjectType>;
    using dict_t = std::unordered_map<std::string, ObjectType>;
    using func_t = std::function<ObjectType(list_t, dict_t)>;

    template<typename Mapping>
    static ObjectType call(const Mapping& scope, const crt::expression& expr)
    {
        auto self = std::unique_ptr<DynamicObject>();
        auto head = var();
        auto args = Array<var>();

        for (const auto& part : expr)
        {
            if (part == expr.front())
            {
                head = scope.at (part.sym());
            }
            else if (expr.key().empty())
            {
                args.add (part.resolve<ObjectType, VarCallAdapter> (scope));
            }
            else
            {
                self->setProperty (String (expr.key()), part.resolve<ObjectType, VarCallAdapter> (scope));
            }
        }
        auto f = head.getNativeFunction();
        auto a = var::NativeFunctionArgs (self.release(), args.begin(), args.size());
        return f(a);
    }

    template<typename Mapping>
    static const ObjectType& at (const Mapping& scope, const std::string& key)
    {
        return scope.at(key);
    }

    static ObjectType convert (const crt::expression::none&) { return var(); }
    static ObjectType convert (const int& value) { return value; }
    static ObjectType convert (const double& value) { return value; }
    static ObjectType convert (const std::string& value) { return String (value); }
};




// ============================================================================
using Kernel = crt::kernel<var, VarCallAdapter>;




// ============================================================================
class WatchedFile
{
public:
    WatchedFile();
    WatchedFile (const String& url);
    bool hasChanged();
    void setData (const var& dataObject);
    StringArray getDataNames() const;
    var getDataItem (const String& key) const;
    File getFile() const;

    // ========================================================================
    struct Status
    {
        Status();
        Status (File file);
        bool refreshFromDisk(); /**< Updates the status and returns true if there was a change. */
        File file;
        Time modified;
        bool existed = false;
    };
private:
    Status status;
    var data; /**< Must be a DynamicObject */
};




// ============================================================================
class Runtime
{
public:

    // ========================================================================
    struct Flags
    {
        enum {
            locked = 8,
            isfile = 16,
        };
    };

    // ========================================================================
    struct Symbols
    {
        static crt::expression file;
    };

    // ========================================================================
    static String getSummary (const var& value);
    static bool isContainer (const var& value);
    static bool hasAttributes (const var& value);
    static bool checkAttribute (const var& value, const String& key);
    static long getFlags (const crt::expression& expr);

    // ========================================================================
    template<typename T> class DataTypeInfo {};

    // ========================================================================
    class GenericData : public ReferenceCountedObject
    {
    public:
        virtual String getSummary() { return String(); }
        virtual StringArray getPropertyNames() { return {}; }
        virtual var getProperty (const String& key) { return var(); }

        static GenericData* cast (const var& value)
        {
            return dynamic_cast<Runtime::GenericData*> (value.getObject());
        }
    };

    // ========================================================================
    template<typename T>
    struct Data : public GenericData
    {
    public:
        Data() {}
        Data (const T& value) : value (value) {}

        static T& check (const var& value)
        {
            if (auto d = dynamic_cast<Data*> (value.getObject()))
            {
                return d->value;
            }
            throw std::invalid_argument ("bad cast to Runtime::Data");
        }

        String getSummary() override { return info.getSummary (value); }
        StringArray getPropertyNames() override { return info.getPropertyNames (value); }
        var getProperty (const String& key) override { return info.getProperty (value, key); }

        T value;
        DataTypeInfo<T> info;
        JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(Data)
    };

    template<typename T>
    static var data (const T& value)
    {
        return new Data<T> (value);
    }

    // ========================================================================
    static var list (var::NativeFunctionArgs args);
    static var dict (var::NativeFunctionArgs args);
    static var item (var::NativeFunctionArgs args);
    static var attr (var::NativeFunctionArgs args);
    static var add (var::NativeFunctionArgs args);
    static var sub (var::NativeFunctionArgs args);
    static var mul (var::NativeFunctionArgs args);
    static var div (var::NativeFunctionArgs args);
    static var file (var::NativeFunctionArgs args);
    static var loadtxt (var::NativeFunctionArgs args);
};




// ============================================================================
template<>
class Runtime::DataTypeInfo<WatchedFile>
{
public:
    String getSummary (const WatchedFile& f) const { return f.getFile().getFileName(); }
    StringArray getPropertyNames (const WatchedFile& f) { return f.getDataNames(); }
    var getProperty (const WatchedFile& f, const String& key) { return f.getDataItem (key); }
};

template<>
class Runtime::DataTypeInfo<nd::ndarray<double, 1>>
{
public:
    String getSummary (const nd::ndarray<double, 1>& A) const { return "Array[" + std::to_string (A.shape()[0]) + "]"; }
    StringArray getPropertyNames (const nd::ndarray<double, 1>&) { return {}; }
    var getProperty (const nd::ndarray<double, 1>&, const String& key) { return var(); }
};
