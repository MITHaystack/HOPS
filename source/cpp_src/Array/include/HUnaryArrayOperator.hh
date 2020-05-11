#ifndef HUnaryArrayOperator_H__
#define HUnaryArrayOperator_H__

#include "HArrayOperator.hh"

namespace hops{

/**
*
*@file HUnaryArrayOperator.hh
*@class HUnaryArrayOperator
*@brief
*@details
*
*<b>Revision History:<b>
*Date Name Brief Description
*Tue Sep 25 10:25:37 EDT 2012 J. Barrett (barrettj@mit.edu) First Version
*
*/


template<typename ArrayType, unsigned int NDIM>
class HUnaryArrayOperator: public HArrayOperator<ArrayType, NDIM>
{
    public:
        HUnaryArrayOperator():fInput(NULL),fOutput(NULL){;};
        virtual ~HUnaryArrayOperator(){;};

        virtual void SetInput(HArrayWrapper<ArrayType, NDIM>* in){fInput = in;};
        virtual void SetOutput(HArrayWrapper<ArrayType, NDIM>* out){fOutput = out;};

        virtual HArrayWrapper<ArrayType,NDIM>* GetInput(){return fInput;};
        virtual HArrayWrapper<ArrayType,NDIM>* GetOutput(){return fOutput;};

        virtual void Initialize(){;};

        virtual void ExecuteOperation() = 0;

    protected:

        HArrayWrapper<ArrayType, NDIM>* fInput;
        HArrayWrapper<ArrayType, NDIM>* fOutput;

};

}//end of namespace


#endif /* __HUnaryArrayOperator_H__ */
