#ifndef MHO_OpenCLComplexPointwiseMultiply_HH__
#define MHO_OpenCLComplexPointwiseMultiply_HH__

#include <algorithm>

#include "MHO_Message.hh"
#include "MHO_NDArrayWrapper.hh"
#include "MHO_BinaryOperator.hh"


#include "MHO_OpenCLInterface.hh"
#include "MHO_OpenCLKernelBuilder.hh"




/*!
*@file MHO_OpenCLComplexPointwiseMultiply.hh
*@class MHO_OpenCLComplexPointwiseMultiply
*@author J. Barrett - barrettj@mit.edu 
*
*@date
*@brief
*/


namespace hops
{

class MHO_OpenCLComplexPointwiseMultiply: public MHO_BinaryOperator<
    MHO_NDArrayWrapper< std::complex<XFloatType>, RANK >,
    MHO_NDArrayWrapper< std::complex<XFloatType>, RANK >,
    MHO_NDArrayWrapper< std::complex<XFloatType>, RANK > >
{
    public:

        MHO_OpenCLComplexPointwiseMultiply():
            fInitialized(false)
        {};

        virtual ~MHO_OpenCLComplexPointwiseMultiply(){};

        virtual bool Initialize() override
        {

        }

        virtual bool Execute() override
        {

        }


    private:

        bool fInitialized;

};

}


#endif /*! MHO_OpenCLComplexPointwiseMultiply_H__ */
