#include <fstream>
#include "Loaders.hpp"
#include "AsciiLoader.hpp"
#include "NumericData.hpp"
#include "Kernel/Builtin.hpp"
using namespace mcl;




//==============================================================================
Object Loaders::load_txt (const Object::List& args, const Object::Dict&)
{
    auto fname = Builtin::check<std::string> (args, 0);

    std::fstream input (fname);
    AsciiLoader loader (input);

    if (! loader.getStatusMessage().empty())
    {
        throw std::runtime_error (loader.getStatusMessage());
    }
    auto columns = Object::dict();

    for (int n = 0; n < loader.getNumColumns(); ++n)
    {
        auto user = std::make_shared<ArrayDouble1> (loader.getColumnData(n));
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
