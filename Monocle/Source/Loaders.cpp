#include <fstream>
#include "Loaders.hpp"
#include "AsciiLoader.hpp"
#include "NumericData.hpp"
using namespace mcl;




//==============================================================================
template<typename T>
static const T& check (const Object::List& args, int index)
{
    try {
        return args.at(0).get<T>();
    }
    catch (const std::out_of_range& e) {
        throw std::runtime_error ("missing argument at index " + std::to_string (index));
    }
    catch (const mpark::bad_variant_access& e) {
        throw std::runtime_error ("wrong data type ("
                                  + std::string (1, args.at(0).type())
                                  + ") at index "
                                  + std::to_string (index));
    }
}




//==============================================================================
Object Loaders::load_txt (const Object::List& args, const Object::Dict&)
{
    auto fname = check<std::string> (args, 0);

    std::fstream input (fname);
    AsciiLoader loader (input);

    if (! loader.getStatusMessage().empty())
        throw std::runtime_error (loader.getStatusMessage());

    auto columns = Object::dict();

    for (int n = 0; n < loader.getNumColumns(); ++n)
    {
        auto user = std::make_shared<NumericArrayDouble1> (loader.getColumnData(n));
        columns[loader.getColumnName(n)] = Object::data (user);
    }
    return columns;
}




//==============================================================================
Object::Dict Loaders::loaders()
{
	auto m = Object::Dict();
    m["load-txt"] = Object::Func (load_txt);
	return m;
}
