#ifndef MHONamed_HH__
#define MHONamed_HH__

/*
*File: MHONamed.hh
*Class: MHONamed
*Author: J. Barrett
*Email: barrettj@mit.edu
*Date: 2020-05-15T20:21:32.924Z
*Description:
*/

#include <string>

namespace hops
{

class MHONamed
{
    public:

        MHONamed():fName(""){};
        MHONamed(const MHONamed& obj);
        MHONamed& operator=(const MHONamed& lhs) = default;
        virtual ~MHONamed() = default;

        bool IsNamed(const std::string& name) const;
        const std::string& GetName() const;
        void SetName(std::string name);

    private:

        std::string fName;
};

}  // namespace hops

#endif /* end of include guard: MHONamed */
