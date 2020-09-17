#ifndef HkInterval_HH__
#define HkInterval_HH__

/*
*File: HkInterval.hh
*Class: HkInterval
*Author: J. Barrett
*Email: barrettj@mit.edu
*Date:
*Description:
*/

#include <limits>
#include <utility>

namespace hops
{

class HkInterval
{
    public:

        HkInterval();
        HkInterval(std::size_t lower_bound, std::size_t upper_bound);
        HkInterval(const HkInterval& copy);
        virtual ~HkInterval();

        void SetInterval(std::size_t lower_bound, std::size_t upper_bound);
        void SetInterval(std::pair<std::size_t,std::size_t> lower_upper);
        std::pair<std::size_t,std::size_t> GetInterval() const;

        void SetLowerBound(std::size_t low);
        void SetUpperBound(std::size_t up);
        std::size_t GetLowerBound() const;
        std::size_t GetUpperBound() const;

        HkInterval& operator=(const HkInterval& rhs)
        {
            if(this != &rhs)
            {
                fLowerBound = rhs.fLowerBound;
                fUpperBound = rhs.fUpperBound;
            }
        }

    private:

        void SetIntervalImpl(std::size_t low, std::size_t up);

        std::size_t fLowerBound;
        std::size_t fUpperBound;

};

}

#endif /* end of include guard: HkInterval */
