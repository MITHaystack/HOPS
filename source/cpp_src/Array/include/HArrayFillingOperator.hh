#ifndef HArrayFillingOperator_H__
#define HArrayFillingOperator_H__

#include "HArrayOperator.hh"
#include <iostream>

namespace hops{

/**
*
*@file HArrayFillingOperator.hh
*@class HArrayFillingOperator
*@brief
*@details
*
*<b>Revision History:<b>
*Date Name Brief Description
*Tue Sep 25 10:25:37 EDT 2012 J. Barrett (barrettj@mit.edu) First Version
*
*/


template< typename T, unsigned int NDIM>
class HArrayFillingOperator: public HArrayOperator<T,NDIM>
{
    public:
        HArrayFillingOperator():fOutput(NULL){;};
        virtual ~HArrayFillingOperator(){;};

        virtual void SetOutput(HArrayWrapper<T,NDIM>* out)
        {
            fOutput = out;
        };

        virtual HArrayWrapper<T,NDIM>* GetOutput(){return fOutput;};

        virtual void Initialize(){;};

        virtual void ExecuteOperation() = 0;

    protected:

        HArrayWrapper<T,NDIM>* fOutput;

};

}//end of namespace


#endif /* __HArrayFillingOperator_H__ */
