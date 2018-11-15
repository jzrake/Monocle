#pragma once
#include "Object.hpp"

namespace mcl { class Builtin; }




// ============================================================================
/**
Class that provides core-level object methods. The functions available through
this class may be loaded into a scope in order to evaluate basic expressions.
*/
class mcl::Builtin
{
public:
    /** Return a pack of basic data structure factories: list, dict, etc. */
    static Object::Dict structures();

    /** Return a pack of basic arithmetic functions: add, sub, mul, div, pow. */
    static Object::Dict arithmetic();

    /** Return a pack of all trig functions in the standard library. */
    static Object::Dict trigonometric();

    /** Return a pack of functions that creates and manipulates arrays. */
    static Object::Dict array();
};
