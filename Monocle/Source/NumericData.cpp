#include <vector>
#include "NumericData.hpp"




//==============================================================================
NumericArrayDouble1::NumericArrayDouble1() {}
NumericArrayDouble1::NumericArrayDouble1 (nd::ndarray<double, 1> array) : array (array) {}
NumericArrayDouble1::NumericArrayDouble1 (const std::vector<double>& vec) : array (int (vec.size()))
{
    std::memcpy (&array(0), &vec[0], vec.size() * sizeof (double));
}

std::string NumericArrayDouble1::type() const
{
    return "NumericArrayDouble1";
}

std::string NumericArrayDouble1::describe() const
{
    return "double [" + std::to_string (array.shape()[0]) + "]";
}

std::string NumericArrayDouble1::serialize() const
{
    return "";
}

bool NumericArrayDouble1::load (const std::string&)
{
    return false;
}
