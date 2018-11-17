#pragma once
#include "Kernel/UserData.hpp"
#include "3rdParty/ndarray/ndarray.hpp"




// ============================================================================
class NumericArrayDouble1 : public mcl::UserData
{
public:
    NumericArrayDouble1();
    NumericArrayDouble1 (nd::ndarray<double, 1> array);
    NumericArrayDouble1 (const std::vector<double>& vec);
    std::string type() const override;
    std::string describe() const override;
    std::string serialize() const override;
    bool load (const std::string&) override;
    nd::ndarray<double, 1> get();
private:
    nd::ndarray<double, 1> array;
};
