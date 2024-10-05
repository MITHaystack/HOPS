#ifndef MHO_NormFX_HH__
#define MHO_NormFX_HH__

#include <cmath>
#include <complex>

#include "MHO_ContainerDefinitions.hh"
#include "MHO_TableContainer.hh"
#include "MHO_UnaryOperator.hh"


namespace hops
{

/*!
 *@file MHO_NormFX.hh
 *@class MHO_NormFX
 *@author J. Barrett - barrettj@mit.edu
 *@date Fri Jul 9 11:47:00 2021 -0400
 *@brief interface for various types of norm_fx operators (unary op on visibilities that accepts weights)
 */

class MHO_NormFX: public MHO_UnaryOperator< visibility_type >
{
    public:
        MHO_NormFX():fWeights(nullptr){};
        virtual ~MHO_NormFX(){};

        void SetWeights(weight_type* w){fWeights = w;}

    protected:
        using XArgType = visibility_type;

        virtual bool InitializeInPlace(XArgType* in) = 0;
        virtual bool InitializeOutOfPlace(const XArgType* in, XArgType* out) = 0;

        virtual bool ExecuteInPlace(XArgType* in)  = 0;
        virtual bool ExecuteOutOfPlace(const XArgType* in, XArgType* out) = 0;

        weight_type* fWeights;

};

} // namespace hops

#endif /*! end of include guard: MHO_NormFX */
