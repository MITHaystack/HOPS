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

/**
 * @brief Class MHO_NormFX
 */
class MHO_NormFX: public MHO_UnaryOperator< visibility_type >
{
    public:
        MHO_NormFX(): fWeights(nullptr){};
        virtual ~MHO_NormFX(){};

        /**
         * @brief Setter for weights
         * 
         * @param w Input pointer to weight_type array
         */
        void SetWeights(weight_type* w) { fWeights = w; }

    protected:
        using XArgType = visibility_type;

        /**
         * @brief Function InitializeInPlace
         * 
         * @param in (XArgType*)
         * @return Return value (bool)
         * @note This is a virtual function.
         */
        virtual bool InitializeInPlace(XArgType* in) = 0;
        /**
         * @brief Function InitializeOutOfPlace
         * 
         * @param in (const XArgType*)
         * @param out (XArgType*)
         * @return Return value (bool)
         * @note This is a virtual function.
         */
        virtual bool InitializeOutOfPlace(const XArgType* in, XArgType* out) = 0;

        /**
         * @brief Function ExecuteInPlace
         * 
         * @param in (XArgType*)
         * @return Return value (bool)
         * @note This is a virtual function.
         */
        virtual bool ExecuteInPlace(XArgType* in) = 0;
        /**
         * @brief Function ExecuteOutOfPlace
         * 
         * @param in (const XArgType*)
         * @param out (XArgType*)
         * @return Return value (bool)
         * @note This is a virtual function.
         */
        virtual bool ExecuteOutOfPlace(const XArgType* in, XArgType* out) = 0;

        weight_type* fWeights;
};

} // namespace hops

#endif /*! end of include guard: MHO_NormFX */
