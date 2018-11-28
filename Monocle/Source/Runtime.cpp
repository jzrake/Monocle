#include <fstream>
#include "Runtime.hpp"
#include "Numerical/AsciiLoader.hpp"




// ============================================================================
WatchedFile::WatchedFile()
{
    data = new DynamicObject;
}

WatchedFile::WatchedFile (const String& url)
{
    if (! File::isAbsolutePath (url))
    {
        throw std::invalid_argument ("invalid file URL");
    }
    status = Status (url);
    data = new DynamicObject;
}

bool WatchedFile::hasChanged()
{
    return status.refreshFromDisk();
}

void WatchedFile::setData (const var& dataObject)
{
    jassert(dataObject.getDynamicObject());
    data = dataObject;
}

StringArray WatchedFile::getDataNames() const
{
    StringArray res;

    for (const auto& p : data.getDynamicObject()->getProperties())
        res.add (p.name.toString());

    return res;
}

var WatchedFile::getDataItem (const String& key) const
{
    return data.getProperty (key, var());
}

File WatchedFile::getFile() const
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
    if (! args.arguments[1].isString())
        throw std::invalid_argument ("attr argument 2 must be a string");
    return checkAttribute (args.arguments[0], args.arguments[1]);
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

    auto path = args.arguments[0].toString();
    auto file = std::make_unique<Data<WatchedFile>> (path);

    if (args.thisObject["filter"].isMethod())
    {
        file->value.setData (args.thisObject.call ("filter", args.arguments[0]));
    }
    return file.release();
}




// ============================================================================
var Runtime::loadtxt (var::NativeFunctionArgs args)
{
    if (args.numArguments != 1)
    {
        throw std::invalid_argument ("loadtxt: no argument");
    }
    if (! File::isAbsolutePath (args.arguments[0].toString()))
    {
        throw std::invalid_argument ("loadtxt: invalid file URL");
    }

    auto fstream = std::fstream (args.arguments[0].toString().getCharPointer());
    auto loader  = mcl::AsciiLoader (fstream);
    auto table   = std::make_unique<DynamicObject>();

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
String Runtime::getSummary (const var& value)
{
    if (auto object = value.getDynamicObject())
    {
        return "[Dict]";
    }
    if (auto object = value.getArray())
    {
        return "[List]";
    }
    if (auto object = dynamic_cast<GenericData*> (value.getObject()))
    {
        return object->getSummary();
    }
    return value.toString();
}

bool Runtime::isContainer (const var& value)
{
    if (value.isArray() || value.getDynamicObject())
    {
        return true;
    }
    if (auto data = GenericData::cast (value))
    {
        return ! data->getPropertyNames().isEmpty();
    }
    return false;
}

bool Runtime::hasAttributes (const var& value)
{
    if (value.getDynamicObject())
    {
        return true;
    }
    if (auto data = GenericData::cast (value))
    {
        return ! data->getPropertyNames().isEmpty();
    }
    return false;
}

bool Runtime::checkAttribute (const var& value, const String& key)
{
    auto error_str = [&] ()
    {
        return "object "
        + getSummary (value).toStdString()
        + " has no attribute "
        + key.toStdString();
    };

    if (auto object = value.getDynamicObject())
    {
        if (! object->hasProperty (key))
        {
            throw std::range_error (error_str());
        }
        return object->getProperty (key);
    }
    else if (auto object = GenericData::cast (value))
    {
        if (! object->getPropertyNames().contains (key))
        {
            throw std::range_error (error_str());
        }
        return object->getProperty (key);
    }
    throw std::range_error (error_str());
}
