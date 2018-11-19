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
    static Object::Dict builtin();
    static Object item (const Object::List&, const Object::Dict&);
    static Object attr (const Object::List&, const Object::Dict&);
    static Object list (const Object::List&, const Object::Dict&);
    static Object dict (const Object::List&, const Object::Dict&);
    static Object join (const Object::List&, const Object::Dict&);
    static Object merge (const Object::List&, const Object::Dict&);
    static Object range (const Object::List&, const Object::Dict&);

    /** Return a pack of basic arithmetic functions: add, sub, mul, div, pow. */
    static Object::Dict arithmetic();

    /** Return a pack of all trig functions in the standard library. */
    static Object::Dict trigonometric();

    /** Return a pack of functions that creates and manipulates arrays. */
    static Object::Dict array();




    //==============================================================================
    template<typename T>
    static const T& check (const Object::List& args, int index)
    {
        try {
            return args.at (index).get<T>();
        }
        catch (const std::out_of_range& e)
        {
            throw std::runtime_error ("missing argument at index " + std::to_string (index));
        }
        catch (const mpark::bad_variant_access& e)
        {
            throw std::runtime_error ("wrong data type ("
                                      + std::string (1, args.at (index).type())
                                      + ") at index "
                                      + std::to_string (index));
        }
    }

    template<typename T>
    static T& check_user_data (const Object::List& args, int index)
    {
        try {
            if (auto res = dynamic_cast<T*>(args.at (index).get<Object::Data>().v.get()))
            {
                return *res;
            }
            throw mpark::bad_variant_access();
        }
        catch (const std::out_of_range& e)
        {
            throw std::runtime_error ("missing argument at index " + std::to_string (index));
        }
        catch (const mpark::bad_variant_access& e)
        {
            throw std::runtime_error ("wrong data type ("
                                      + std::string (1, args.at(0).type())
                                      + ") at index "
                                      + std::to_string (index));
        }
    }
};
