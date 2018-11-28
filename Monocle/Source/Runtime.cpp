#include "Runtime.hpp"




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
    return Array<var>(args.arguments + 1, args.numArguments - 1);
}

var Runtime::dict (var::NativeFunctionArgs args)
{
    return args.thisObject;
}

var Runtime::item (var::NativeFunctionArgs args)
{
    if (args.numArguments - 1 != 2)
        throw std::invalid_argument ("item requires two arguments");
    if (! args.arguments[1].isArray())
        throw std::invalid_argument ("item argument 1 must be a list");
    if (! args.arguments[2].isInt())
        throw std::invalid_argument ("item argument 2 must be an int");
    return args.arguments[1][int (args.arguments[2])];
}

var Runtime::attr (var::NativeFunctionArgs args)
{
    if (args.numArguments - 1 != 2)
        throw std::invalid_argument ("attr requires two arguments");
    if (! args.arguments[1].getDynamicObject())
        throw std::invalid_argument ("attr argument 1 must be a dict");
    if (! args.arguments[2].isString())
        throw std::invalid_argument ("attr argument 2 must be a string");
    return args.arguments[1].getProperty (args.arguments[2].toString(), var());
}

var Runtime::add (var::NativeFunctionArgs args)
{
    double res = 0.0;

    for (int n = 1; n < args.numArguments; ++n)
        res += double (args.arguments[n]);
    return res;
}

var Runtime::sub (var::NativeFunctionArgs args)
{
    double res = 0.0;

    for (int n = 1; n < args.numArguments; ++n)
        res -= double (args.arguments[n]);
    return res;
}

var Runtime::mul (var::NativeFunctionArgs args)
{
    double res = 1.0;

    for (int n = 1; n < args.numArguments; ++n)
        res *= double (args.arguments[n]);
    return res;
}

var Runtime::div (var::NativeFunctionArgs args)
{
    double res = 1.0;

    for (int n = 1; n < args.numArguments; ++n)
        res /= double (args.arguments[n]);
    return res;
}

var Runtime::file (var::NativeFunctionArgs args)
{
    if (args.numArguments == 1)
    {
        return var();
    }

    var f = new DynamicObject;
    auto path = args.arguments[1].toString();
    f.getDynamicObject()->setProperty ("status", new Data<WatchedFile> (path));
    f.getDynamicObject()->setProperty ("data", var());
    return f;
}
