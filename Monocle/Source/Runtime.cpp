#include "Runtime.hpp"




var Runtime::list (var::NativeFunctionArgs args)
{
    return Array<var>(args.arguments, args.numArguments);
}

var Runtime::dict (var::NativeFunctionArgs args)
{
    return args.thisObject;
}

var Runtime::add (var::NativeFunctionArgs args)
{
    double res = 0.0;

    for (int n = 0; n < args.numArguments; ++n)
        res += double (args.arguments[n]);
    return res;
}
