#ifndef MHO_SBDTableGenerator_HH__
#define MHO_SBDTableGenerator_HH__


#include <cmath>
#include <complex>

#include "MHO_TableContainer.hh"
#include "MHO_ContainerDefinitions.hh"

#include "MHO_TransformingOperator.hh"

#include "MHO_NaNMasker.hh"
#include "MHO_FunctorBroadcaster.hh"
#include "MHO_ComplexConjugator.hh"
// #include "MHO_CyclicRotator.hh"
// #include "MHO_SubSample.hh"
// #include "MHO_EndZeroPadder.hh"
// #include "MHO_MultidimensionalFastFourierTransform.hh"

// #ifdef HOPS_USE_FFTW3
// #include "MHO_MultidimensionalFastFourierTransformFFTW.hh"
// #endif

namespace hops
{

/*!
*@file MHO_SBDTableGenerator.hh
*@class MHO_SBDTableGenerator
*@author J. Barrett - barrettj@mit.edu
*@date Fri Jul 9 11:47:00 2021 -0400
*@brief implements the conversion of the input visibility array into something 
* which can be transformed into singleband delay space. This is because of the 
* way in which the fourfit algorithm zero-pads the data, but also because if we 
* have double-sidband channels they need to be merged into the same array 
* at this point. Note that 'visibility_type' and 'sbd_type' are actually the 
* same underlying type of table container, but this semantics is useful for 
* keeping track of what is going on.
*
*/ 



class MHO_SBDTableGenerator: public MHO_TransformingOperator< visibility_type, sbd_type >
{
    public:
        MHO_SBDTableGenerator();
        virtual ~MHO_SBDTableGenerator();

    protected:

        using XArgType1 = visibility_type;
        using XArgType2 = sbd_type;

        virtual bool InitializeImpl(const XArgType1* in, XArgType2* out);
        virtual bool ExecuteImpl(const XArgType1* in, XArgType2* out);

    private:

        std::size_t fInDims[VIS_NDIM];
        std::size_t fWorkDims[VIS_NDIM];
        std::size_t fOutDims[VIS_NDIM];

        typedef MHO_NaNMasker<visibility_type> nanMaskerType;
        typedef MHO_ComplexConjugator<visibility_type> conjType;

        MHO_FunctorBroadcaster<visibility_type, nanMaskerType> fNaNBroadcaster;

        //function to resize the sbd array if needed -- no double-sideband data
        void ConditionallyResizeOutput(const XArgType1* in, XArgType2* out);

        //function to resize the sbd array if needed with double-sideband data
        void ConditionallyResizeOutputDSB(const XArgType1* in, XArgType2* out);


        bool fInitialized;

};


}


#endif /*! end of include guard: MHO_SBDTableGenerator */
