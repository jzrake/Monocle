#pragma once
#include "Kernel/Object.hpp"




// ============================================================================
class Loaders
{
public:
    using Object = mcl::Object;

    static Object::Dict loaders();
    static Object load_txt (const Object::List& args, const Object::Dict&);
};
