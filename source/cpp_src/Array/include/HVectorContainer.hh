#ifndef HVectorContainer_HH__
#define HVectorContainer_HH__

#include "HArrayWrapper.hh"

namespace hops{


template< typename XValueType, typename XUnitType >
class VectorContainer: public HArrayWrapper< XValueType, 1 >
{
    public:
        std::string fName;
        VectorContainer();
        virtual ~VectorContainer();
        //...TBD impl...
    private:
        std::string fName;
        XUnitType fUnit;
        std::vector< XValueType > fData;
};

}//end of hops namespace

#endif /* end of include guard: HVectorContainer_HH__ */
