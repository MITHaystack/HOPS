#ifndef MHO_DelayRate_HH__
#define MHO_DelayRate_HH__

#include <cmath>
#include <complex>

#include "MHO_ContainerDefinitions.hh"
#include "MHO_TableContainer.hh"

#include "MHO_BinaryOperator.hh"
#include "MHO_CyclicRotator.hh"
#include "MHO_EndZeroPadder.hh"
#include "MHO_MultidimensionalFastFourierTransform.hh"
#include "MHO_SubSample.hh"

#ifdef HOPS_USE_FFTW3
    #include "MHO_MultidimensionalFastFourierTransformFFTW.hh"
#endif

namespace hops
{

/*!
 *@file MHO_DelayRate.hh
 *@class MHO_DelayRate
 *@author J. Barrett - barrettj@mit.edu
 *@date Fri Feb 3 15:13:40 2023 -0500
 *@brief implements the delay rate search
 */

/**
 * @brief Class MHO_DelayRate
 */
class MHO_DelayRate: public MHO_BinaryOperator< visibility_type, weight_type, sbd_type >
{
    public:
        MHO_DelayRate();
        virtual ~MHO_DelayRate();

        /**
         * @brief Setter for reference frequency
         * 
         * @param ref_freq New reference frequency value in Hertz
         */
        void SetReferenceFrequency(double ref_freq) { fRefFreq = ref_freq; };

        /**
         * @brief Getter for delay rate search space size
         * 
         * @return Delay rate search space size as an integer
         */
        int GetDelayRateSearchSpaceSize() const { return fDRSPSize; }

        /**
         * @brief Calculates the search space size based on input size
         * 
         * @param input_size Input size for which to calculate the search space
         * @return Calculated search space size as an unsigned integer
         */
        unsigned int CalculateSearchSpaceSize(unsigned int input_size);

    protected:
        using XArgType1 = visibility_type;
        using XArgType2 = weight_type;
        using XArgType3 = sbd_type;

        /**
         * @brief Initializes MHO_DelayRate with input data and prepares for delay rate calculation.
         * 
         * @param in1 Input data of type XArgType1
         * @param in2 Additional input data of type XArgType2
         * @param out Output workspace of type XArgType3
         * @return True if initialization is successful, false otherwise
         * @note This is a virtual function.
         */
        virtual bool InitializeImpl(const XArgType1* in1, const XArgType2* in2, XArgType3* out) override;
        /**
         * @brief Executes MHO_DelayRate operations: zero padding, FFT, cyclic rotation, and interpolation.
         * 
         * @param in1 Input data for interpolation
         * @param in2 Input data weights
         * @param out Output padded array
         * @return True if all operations succeed, false otherwise
         * @note This is a virtual function.
         */
        virtual bool ExecuteImpl(const XArgType1* in1, const XArgType2* in2, XArgType3* out) override;

    private:
        std::size_t fInDims[VIS_NDIM];
        std::size_t fOutDims[VIS_NDIM];

        /**
         * @brief Applies data weights from input array to output array elements.
         * 
         * @param in2 Input weight array of type XArgType2
         * @param out Output data array of type XArgType3
         */
        void ApplyDataWeights(const XArgType2* in2, XArgType3* out);
        /**
         * @brief Conditionally resizes output array dimensions if they differ from required dimensions.
         * 
         * @param dims Input dimension sizes
         * @param size Desired size for time axis
         * @param out Output array to be conditionally resized
         */
        void ConditionallyResizeOutput(const std::size_t* dims, std::size_t size, XArgType3* out);

        /**
         * @brief Applies linear interpolation and modifies delay rate axis for input data.
         * 
         * @param in1 Input data array of type XArgType1
         * @param out Output data array of type XArgType3
         */
        void ApplyInterpolation(const XArgType1* in1, XArgType3* out);

#ifdef HOPS_USE_FFTW3
        using FFT_ENGINE_TYPE = MHO_MultidimensionalFastFourierTransformFFTW< visibility_type >;
#else
        using FFT_ENGINE_TYPE = MHO_MultidimensionalFastFourierTransform< visibility_type >;
#endif

        MHO_SubSample< sbd_type > fSubSampler;
        MHO_CyclicRotator< sbd_type > fCyclicRotator;

        MHO_EndZeroPadder< visibility_type > fZeroPadder;
        FFT_ENGINE_TYPE fFFTEngine;

        int fDRSPSize;
        double fRefFreq;

        bool fInitialized;
};

} // namespace hops

#endif /*! end of include guard: MHO_DelayRate */
