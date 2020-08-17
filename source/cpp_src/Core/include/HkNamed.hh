#ifndef HkNamed_HH__
#define HkNamed_HH__

/*
*File: HkNamed.hh
*Class: HkNamed
*Author: J. Barrett
*Email: barrettj@mit.edu
*Date: 2020-05-15T20:21:32.924Z
*Description:
*/

#include <string>

namespace hops
{

class HkNamed
{
    public:

        HkNamed():fName(""){};
        HkNamed(const HkNamed& obj);
        HkNamed& operator=(const HkNamed& lhs) = default;
        virtual ~HkNamed() = default;

        bool IsNamed(const std::string& name) const;
        const std::string& GetName() const;
        void SetName(std::string name);

    private:

        std::string fName;
};

}  // namespace hops

#endif /* end of include guard: HkNamed */
