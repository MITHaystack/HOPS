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

/**
 * @brief Class MHO_SBDTableGenerator
 */
class MHO_SBDTableGenerator: public MHO_TransformingOperator< visibility_type, sbd_type >
{
    public:
        MHO_SBDTableGenerator();
        virtual ~MHO_SBDTableGenerator();

    protected:
        using XArgType1 = visibility_type;
        using XArgType2 = sbd_type;

        /**
         * @brief Initializes SBD table generator with raw visibilities and resizes output if needed.
         *
         * @param in Input raw visibilities
         * @param out Output single-band-delay table (workspace/output for normfx)
         * @return True if initialization was successful, false otherwise
         * @note This is a virtual function.
         */
        virtual bool InitializeImpl(const XArgType1* in, XArgType2* out);
        /**
         * @brief Checks if initialized and returns true if so, false otherwise.
         *
         * @param in Input raw visibilities
         * @param out Output single-band-delay table workspace
         * @return Boolean indicating whether the object is initialized
         * @note This is a virtual function.
         */
        virtual bool ExecuteImpl(const XArgType1* in, XArgType2* out);

    private:
        std::size_t fInDims[VIS_NDIM];
        std::size_t fWorkDims[VIS_NDIM];
        std::size_t fOutDims[VIS_NDIM];

        //function to resize the sbd array if needed -- no double-sideband data
        /**
         * @brief Checks and conditionally resizes output sbd array if needed for NormFX processing.
         *
         * @param in Input raw visibilities
         * @param out Single-band-delay table (workspace/output for NormFX)
         */
        void ConditionallyResizeOutput(const XArgType1* in, XArgType2* out);

        bool fInitialized;
};

} // namespace hops

#endif /*! end of include guard: MHO_SBDTableGenerator */
