#pragma once
#include "JuceHeader.h"
#include "3rdParty/crt-kernel/kernel.hpp"




// ============================================================================
class Runtime
{
public:

    enum KernelFlags
    {
        locked = 8,
    };

    template<typename T>
    struct Data : public ReferenceCountedObject
    {
    public:
        Data() {}
        Data (const T& value) : value (value) {}
        T value;
        JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(Data)
    };

    static var builtin_list (var::NativeFunctionArgs args)
    {
        return Array<var>(args.arguments, args.numArguments);
    }

    static var builtin_dict (var::NativeFunctionArgs args)
    {
        return args.thisObject;
    }
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
        auto self = new DynamicObject;

        for (const auto& kw : kwar)
        {
            self->setProperty (String (kw.first), kw.second);
        }
        auto f = scope.at(key).getNativeFunction();
        auto a = var::NativeFunctionArgs (var (self), &args[0], int (args.size()));
        return f(a);
    }
};
using Kernel = crt::kernel<var, VarCallAdapter>;
