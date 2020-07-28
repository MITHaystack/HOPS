#include "HkNamed.hh"

namespace hops
{

    HkNamed::HNamed():fName(""){};
    HkNamed::HNamed(const HkNamed& obj) : fName(obj.fName) {};

    bool HkNamed::IsNamed(const std::string& name) const
    {
        return fName == name;
    }

    void HkNamed::SetName(std::string name)
    {
        fName = name;
    }

    const std::string& HkNamed::GetName() const
    {
        return fName;
    }

}
