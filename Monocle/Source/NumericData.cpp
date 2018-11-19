#include <vector>
#include "NumericData.hpp"




//==============================================================================
ArrayDouble1::ArrayDouble1() {}
ArrayDouble1::ArrayDouble1 (nd::ndarray<double, 1> array) : array (array) {}
ArrayDouble1::ArrayDouble1 (const std::vector<double>& vec) : array (int (vec.size()))
{
    std::memcpy (&array(0), &vec[0], vec.size() * sizeof (double));
}

nd::ndarray<double, 1>& ArrayDouble1::get()
{
    return array;
}

std::string ArrayDouble1::type() const
{
    return "ArrayDouble1";
}

std::string ArrayDouble1::describe() const
{
    return "double [" + std::to_string (array.shape()[0]) + "]";
}

std::string ArrayDouble1::serialize() const
{
    return "";
}

bool ArrayDouble1::load (const std::string&)
{
    return false;
}
