#include <fstream>
#include "Loaders.hpp"
#include "AsciiLoader.hpp"
#include "NumericData.hpp"




//==============================================================================
mcl::Object Loaders::load_txt (const mcl::Object::List& args, const mcl::Object::Dict&)
{
    std::fstream input (args.at(0).get<std::string>());
    mcl::AsciiLoader loader (input);

    if (! loader.getStatusMessage().empty())
        throw std::runtime_error (loader.getStatusMessage());

    auto columns = mcl::Object::dict();

    for (int n = 0; n < loader.getNumColumns(); ++n)
    {
        auto user = std::make_shared<NumericArrayDouble1> (loader.getColumnData(n));
        columns[loader.getColumnName(n)] = mcl::Object::data (user);
    }
    return columns;
}




//==============================================================================
mcl::Object::Dict Loaders::loaders()
{
	auto m = Object::Dict();
    m["load-txt"] = Object::Func (load_txt);
	return m;
}
