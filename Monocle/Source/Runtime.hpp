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

    // ========================================================================
    template<typename T>
    class DataTypeInfo
    {
    public:
        String getType() const { return "Data"; }
    };

    // ========================================================================
    class GenericData : public ReferenceCountedObject
    {
    public:
        virtual String getType() = 0;
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

        String getType() override
        {
            return info.getType();
        }

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
    String getType() const { return "File Status"; }
};

template<>
class Runtime::DataTypeInfo<nd::ndarray<double, 1>>
{
public:
    String getType() const { return "nd::array<double, 1>"; }
};
