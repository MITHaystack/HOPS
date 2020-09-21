#ifndef HkAxisLabel_HH__
#define HkAxisLabel_HH__

/*
*File: HkAxisLabel.hh
*Class: HkAxisLabel
*Author: J. Barrett
*Email: barrettj@mit.edu
*Date:
*Description:
*/

#include <string>
#include <utility>

#include "HkInterval.hh"

namespace hops
{

template< typename XValueType >
class HkAxisLabel: public HkInterval< std::size_t >
{
    public:

        HkAxisLabel():
            HkInterval(),
            fCategoryName(""),
            fLabelName("")
        {}

        HkAxisLabel(std::string category, std::string label):
            HkInterval(),
            fCategoryName(category),
            fLabelName(label)
        {}

        HkAxisLabel(std::string category, std::string label, std::size_t lower_bound, std::size_t upper_bound):
            HkInterval(lower_bound,upper_bound),
            fCategoryName(category),
            fLabelName(label)
        {};

        HkAxisLabel(const HkAxisLabel& copy):
            HkInterval(copy)
        {
            fCategoryName = copy.fCategoryName;
            fLabelName = copy.fLabelName;
            fValue = copy.fValue;
        }

        virtual ~HkAxisLabel();

        const std::string& GetCategoryName() const;
        void SetCategoryName(std::string name){fCategoryName = name;};

        const std::string& GetLabelName() const;
        void SetLabelName(std::string name){fLabelName = name;};

        void SetValue(const& XValueType value){fValue = value;}
        XValueType GetValue() const {return fValue;};

        HkAxisLabel& operator=(const HkAxisLabel& rhs)
        {
            if(this != &rhs)
            {
                SetIntervalImpl(rhs.fLowerBound, rhs.fUpperBound );
                fCategoryName = rhs.fCategoryName;
                fLabelName = rhs.fLabelName;
                fValue = rhs.fValid;
            }
        }

    private:

        std::string fCategoryName;
        std::string fLabelName;
        XValueType fValue;

};

}

#endif /* end of include guard: HkAxisLabel */
