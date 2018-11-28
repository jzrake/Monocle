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
    static ObjectType call(const Mapping& scope,
                           const std::string& key,
                           const list_t& args,
                           const dict_t& kwar)
    {
        var self = new DynamicObject;

        for (const auto& kw : kwar)
        {
            self.getDynamicObject()->setProperty (String (kw.first), kw.second);
        }
        auto f = scope.at(key).getNativeFunction();
        auto a = var::NativeFunctionArgs (self, &args[1], int (args.size() - 1));
        return f(a);
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
    File getFile();
    void setData (const var& dataObject);
    StringArray getDataNames() const;
    var getDataItem (const String& key) const;

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
    enum KernelFlags
    {
        locked = 8,
    };

    // ========================================================================
    struct Symbols
    {
        static crt::expression file;
    };

    // ========================================================================
    static String summarize (const var& value);
    static bool isContainer (const var& value);
    static bool hasAttributes (const var& value);
    static bool checkAttribute (const var& value, const String& key);

    // ========================================================================
    template<typename T> class DataTypeInfo {};

    // ========================================================================
    class GenericData : public ReferenceCountedObject
    {
    public:
        virtual String getType() { return "Unknown"; }
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

        String getType() override { return info.getType(); }
        StringArray getPropertyNames() override { return info.getPropertyNames (value); }
        var getProperty (const String& key) override { return info.getProperty (value, key); }

        T value;
        DataTypeInfo<T> info;
        JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(Data)
    };

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
    String getType() const { return "WatchedFile"; }
    StringArray getPropertyNames (const WatchedFile& f) { return f.getDataNames(); }
    var getProperty (const WatchedFile& f, const String& key) { return f.getDataItem (key); }
};

template<>
class Runtime::DataTypeInfo<nd::ndarray<double, 1>>
{
public:
    String getType() const { return "nd::array<double, 1>"; }
    StringArray getPropertyNames (const nd::ndarray<double, 1>&) { return {}; }
    var getProperty (const nd::ndarray<double, 1>&, const String& key) { return var(); }
};
