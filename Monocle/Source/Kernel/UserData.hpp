#pragma once
#include <string>

namespace mcl { class UserData; }




// ============================================================================
class mcl::UserData
{
public:
    virtual ~UserData() {}
	virtual std::string type() const = 0;
	virtual std::string describe() const = 0;
	virtual std::string serialize() const = 0;
    virtual bool load (const std::string&) = 0;
};
