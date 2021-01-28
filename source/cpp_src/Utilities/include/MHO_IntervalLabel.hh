#ifndef MHO_IntervalLabel_HH__
#define MHO_IntervalLabel_HH__

/*
*File: MHO_IntervalLabel.hh
*Class: MHO_IntervalLabel
*Author: J. Barrett
*Email: barrettj@mit.edu
*Date:
*Description:
*/

#include <string>
#include <utility>

#include "MHO_Interval.hh"
#include "MHO_MultiTypeMap.hh"

namespace hops
{

//TODO: Make sure this set of types is complete for data-labelling.
//Consider what other types might be needed (float? short? dates?)
typedef MHO_MultiTypeMap< std::string, char, bool, int, double, std::string > MHO_IntervalLabelMap;

class MHO_IntervalLabel: public MHO_Interval< std::size_t >, public MHO_IntervalLabelMap
{
    public:

        MHO_IntervalLabel();
        MHO_IntervalLabel( std::size_t lower_bound, std::size_t upper_bound);
        MHO_IntervalLabel(const MHO_IntervalLabel& copy);
        virtual ~MHO_IntervalLabel();

        bool HasKey(const std::string& key) const;

        MHO_IntervalLabel& operator=(const MHO_IntervalLabel& rhs)
        {
            if(this != &rhs)
            {
                SetIntervalImpl(rhs.fLowerBound, rhs.fUpperBound );
                this->CopyFrom<char>(rhs);
                this->CopyFrom<bool>(rhs);
                this->CopyFrom<int>(rhs);
                this->CopyFrom<double>(rhs);
                this->CopyFrom<std::string>(rhs);
            }
            return *this;
        }

    private:



};

}

#endif /* end of include guard: MHO_IntervalLabel */
