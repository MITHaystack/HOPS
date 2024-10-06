#ifndef MHO_SBDTableGenerator_HH__
#define MHO_SBDTableGenerator_HH__

#include <cmath>
#include <complex>

#include "MHO_ContainerDefinitions.hh"
#include "MHO_TableContainer.hh"
#include "MHO_TransformingOperator.hh"


namespace hops
{

/*!
 *@file MHO_SBDTableGenerator.hh
 *@class MHO_SBDTableGenerator
 *@author J. Barrett - barrettj@mit.edu
 *@date Fri Jul 9 11:47:00 2021 -0400
 *@brief implements the conversion of the input visibility array into something
 * which can be transformed into singleband delay space, all it does is construct 
 * the sbd data container and size it appropriately
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

        typedef MHO_NaNMasker< visibility_type > nanMaskerType;
        typedef MHO_ComplexConjugator< visibility_type > conjType;

        MHO_FunctorBroadcaster< visibility_type, nanMaskerType > fNaNBroadcaster;

        //function to resize the sbd array if needed -- no double-sideband data
        void ConditionallyResizeOutput(const XArgType1* in, XArgType2* out);

        bool fInitialized;
};

} // namespace hops

#endif /*! end of include guard: MHO_SBDTableGenerator */
