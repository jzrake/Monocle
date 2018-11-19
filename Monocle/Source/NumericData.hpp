#pragma once
#include "Kernel/UserData.hpp"
#include "3rdParty/ndarray/ndarray.hpp"




// ============================================================================
class ArrayDouble1 : public mcl::UserData
{
public:
    ArrayDouble1();
    ArrayDouble1 (nd::ndarray<double, 1> array);
    ArrayDouble1 (const std::vector<double>& vec);
    nd::ndarray<double, 1>& get();
    std::string type() const override;
    std::string describe() const override;
    std::string serialize() const override;
    bool load (const std::string&) override;
private:
    nd::ndarray<double, 1> array;
};
