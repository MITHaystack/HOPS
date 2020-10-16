#ifndef MHOIntervalLabel_HH__
#define MHOIntervalLabel_HH__

/*
*File: MHOIntervalLabel.hh
*Class: MHOIntervalLabel
*Author: J. Barrett
*Email: barrettj@mit.edu
*Date:
*Description:
*/

#include <string>
#include <utility>

#include "MHOInterval.hh"
#include "MHOMultiTypeMap.hh"

namespace hops
{

//TODO: Make sure this set of types is complete for data-labelling.
//Consider what other types might be needed (float? short? dates?)
typedef MHOMultiTypeMap< std::string, char, bool, int, double, std::string > MHOIntervalLabelMap;

class MHOIntervalLabel: public MHOInterval< std::size_t >, public MHOIntervalLabelMap
{
    public:

        MHOIntervalLabel();
        MHOIntervalLabel( std::size_t lower_bound, std::size_t upper_bound);
        MHOIntervalLabel(const MHOIntervalLabel& copy);
        virtual ~MHOIntervalLabel();

        MHOIntervalLabel& operator=(const MHOIntervalLabel& rhs)
        {
            if(this != &rhs)
            {
                SetIntervalImpl(rhs.fLowerBound, rhs.fUpperBound );
                this->CopyFrom<char>(rhs);
                this->CopyFrom<bool>(rhs);
                this->CopyFrom<int>(rhs);
                this->CopyFrom<double>(rhs);
            }
            return *this;
        }

    private:



};

}

#endif /* end of include guard: MHOIntervalLabel */
