#include "MHO_Named.hh"

namespace hops
{


    MHO_Named::MHO_Named(const MHO_Named& obj) : fName(obj.fName) {};

    bool MHO_Named::IsNamed(const std::string& name) const
    {
        return fName == name;
    }

    void MHO_Named::SetName(std::string name)
    {
        fName = name;
    }

    const std::string& MHO_Named::GetName() const
    {
        return fName;
    }

}
