#include "HNamed.hh"

namespace hops
{

    HNamed::HNamed():fName(""){};
    HNamed::HNamed(const HNamed& obj) : fName(obj.fName) {};

    bool HNamed::IsNamed(const std::string& name) const
    {
        return fName == name;
    }

    void HNamed::SetName(std::string name)
    {
        fName = name;
    }

    const std::string& HNamed::GetName() const
    {
        return fName;
    }

}
