#pragma once
#include "JuceHeader.h"
#include "3rdParty/crt-kernel/kernel.hpp"




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
    template<typename T>
    struct Data : public ReferenceCountedObject
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

        T value;
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
};




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
        auto a = var::NativeFunctionArgs (self, &args[0], int (args.size()));
        return f(a);
    }

    static ObjectType convert (const crt::expression::none&) { return var(); }
    static ObjectType convert (const int& value) { return value; }
    static ObjectType convert (const double& value) { return value; }
    static ObjectType convert (const std::string& value) { return String (value); }
};
using Kernel = crt::kernel<var, VarCallAdapter>;
