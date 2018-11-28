#include <fstream>
#include "Runtime.hpp"
#include "Numerical/AsciiLoader.hpp"




// ============================================================================
WatchedFile::WatchedFile()
{
}

WatchedFile::WatchedFile (const String& url)
{
    if (! File::isAbsolutePath (url))
    {
        throw std::invalid_argument ("invalid file URL");
    }
    status = Status (url);
}

bool WatchedFile::hasChanged()
{
    return status.refreshFromDisk();
}

File WatchedFile::getFile()
{
    return status.file;
}




// ============================================================================
WatchedFile::Status::Status()
{
}

WatchedFile::Status::Status (File file) : file (file)
{
    refreshFromDisk();
}

bool WatchedFile::Status::refreshFromDisk()
{
    auto last = *this;
    existed = file.existsAsFile();
    modified = file.getLastModificationTime();
    return last.existed != existed || last.modified != modified;
}




// ============================================================================
crt::expression Runtime::Symbols::file = crt::expression::symbol ("file");




// ============================================================================
var Runtime::list (var::NativeFunctionArgs args)
{
    return Array<var>(args.arguments, args.numArguments);
}

var Runtime::dict (var::NativeFunctionArgs args)
{
    return args.thisObject;
}

var Runtime::item (var::NativeFunctionArgs args)
{
    if (args.numArguments != 2)
        throw std::invalid_argument ("item requires two arguments");
    if (! args.arguments[0].isArray())
        throw std::invalid_argument ("item argument 1 must be a list");
    if (! args.arguments[1].isInt())
        throw std::invalid_argument ("item argument 2 must be an int");
    return args.arguments[0][int (args.arguments[1])];
}

var Runtime::attr (var::NativeFunctionArgs args)
{
    if (args.numArguments != 2)
        throw std::invalid_argument ("attr requires two arguments");
    if (! args.arguments[0].getDynamicObject())
        throw std::invalid_argument ("attr argument 1 must be a dict");
    if (! args.arguments[1].isString())
        throw std::invalid_argument ("attr argument 2 must be a string");
    return args.arguments[0].getProperty (args.arguments[1].toString(), var());
}

var Runtime::add (var::NativeFunctionArgs args)
{
    double res = 0.0;

    for (int n = 0; n < args.numArguments; ++n)
        res += double (args.arguments[n]);
    return res;
}

var Runtime::sub (var::NativeFunctionArgs args)
{
    double res = 0.0;

    for (int n = 0; n < args.numArguments; ++n)
        res -= double (args.arguments[n]);
    return res;
}

var Runtime::mul (var::NativeFunctionArgs args)
{
    double res = 1.0;

    for (int n = 0; n < args.numArguments; ++n)
        res *= double (args.arguments[n]);
    return res;
}

var Runtime::div (var::NativeFunctionArgs args)
{
    double res = 1.0;

    for (int n = 0; n < args.numArguments; ++n)
        res /= double (args.arguments[n]);
    return res;
}

var Runtime::file (var::NativeFunctionArgs args)
{
    if (args.numArguments == 0)
    {
        throw std::invalid_argument ("file: no argument");
    }

    auto file = std::make_unique<DynamicObject>();
    auto path = args.arguments[0].toString();

    file->setProperty ("status", new Data<WatchedFile> (path));
    file->setProperty ("data", var());

    if (args.thisObject["filter"].isMethod())
    {
        auto D = args.thisObject.call ("filter", args.arguments[0]);
        file->setProperty ("data", D);
    }
    return file.release();
}




// ============================================================================
var Runtime::loadtxt (var::NativeFunctionArgs args)
{
    if (args.numArguments != 1)
        throw std::invalid_argument ("loadtxt: no argument");
    if (! File::isAbsolutePath (args.arguments[0].toString()))
        throw std::invalid_argument ("loadtxt: invalid file URL");

    auto fstream = std::fstream (args.arguments[0].toString().getCharPointer());
    auto loader = mcl::AsciiLoader (fstream);
    auto table = std::make_unique<DynamicObject>();

    for (int n = 0; n < loader.getNumColumns(); ++n)
    {
        auto x = loader.getColumnData(n);
        auto X = nd::ndarray<double, 1> (int (x.size()));

        for (int i = 0; i < x.size(); ++i)
        {
            X(i) = x[i];
        }
        var c = new Data<nd::ndarray<double, 1>> (X);
        table->setProperty (String (loader.getColumnName(n)), c);
    }
    return table.release();
}




// ============================================================================
String Runtime::summarize (const var& value)
{
    if (auto object = value.getDynamicObject())
    {
        return "{dict}";
    }
    if (auto object = value.getArray())
    {
        return "[list]";
    }
    if (auto object = dynamic_cast<GenericData*> (value.getObject()))
    {
        return object->getType();
    }
    return value.toString();
}
