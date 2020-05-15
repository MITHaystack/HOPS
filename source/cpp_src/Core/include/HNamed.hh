#ifndef HNamed_HH__
#define HNamed_HH__

/*
*File: HNamed.hh
*Class: HNamed
*Author: J. Barrett
*Email: barrettj@mit.edu
*Date: 2020-05-15T20:21:32.924Z
*Description:
*/

#include <string>

namespace hops
{

class HNamed
{
    public:

        HNamed();
        HNamed(const HNamed& obj);
        HNamed& operator=(const HNamed& lhs) = default;
        virtual ~HNamed() = default;

        bool IsNamed(const std::string& name) const;
        const std::string& GetName() const;
        void SetName(std::string name);

    private:

        std::string fName;
};

}  // namespace hops

#endif /* end of include guard: HNamed */
