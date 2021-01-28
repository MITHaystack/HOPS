#ifndef MHO_Named_HH__
#define MHO_Named_HH__

/*
*File: MHO_Named.hh
*Class: MHO_Named
*Author: J. Barrett
*Email: barrettj@mit.edu
*Date: 2020-05-15T20:21:32.924Z
*Description:
*/

#include <string>

namespace hops
{

class MHO_Named
{
    public:

        MHO_Named():fName(""){};
        MHO_Named(const MHO_Named& obj);
        MHO_Named& operator=(const MHO_Named& lhs) = default;
        virtual ~MHO_Named() = default;

        bool IsNamed(const std::string& name) const;
        const std::string& GetName() const;
        void SetName(std::string name);

    private:

        std::string fName;
};

}  // namespace hops

#endif /* end of include guard: MHO_Named */
