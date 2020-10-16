#include "MHONamed.hh"

namespace hops
{


    MHONamed::MHONamed(const MHONamed& obj) : fName(obj.fName) {};

    bool MHONamed::IsNamed(const std::string& name) const
    {
        return fName == name;
    }

    void MHONamed::SetName(std::string name)
    {
        fName = name;
    }

    const std::string& MHONamed::GetName() const
    {
        return fName;
    }

}
