#ifndef MHO_DelayRate_HH__
#define MHO_DelayRate_HH__

#include <cmath>
#include <complex>

#include "MHO_ContainerDefinitions.hh"
#include "MHO_TableContainer.hh"

#include "MHO_BinaryOperator.hh"
#include "MHO_EndZeroPadderOptimized.hh"
#include "MHO_MultidimensionalFastFourierTransform.hh"

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
 *
 * Transforms a single SBD lag slice of the visibilities from the time (AP) domain into
 * the delay-rate domain. The sequence is: zero-pad the time axis by kTimePadFactor,
 * apply the data weights, forward FFT, then resample each channel onto a delay-rate grid
 * which is common to all channels.
 *
 * The last step is somewhat convoluted. The fringe rate seen in a channel is
 * (delay rate) * (that channel's sky frequency), so one physical delay rate falls in a
 * different fringe-rate bin in every channel. Resampling channel c by nu_c/nu_ref removes
 * that dependence, so that output bin k maps to the same delay rate in every channel and
 * MHO_MBDelaySearch may accumulate different channels into a shared rate bin.
 *
 * Note that the axis labels written here are fringe rates *at the reference frequency*,
 * i.e. nu_ref * (delay rate); the division by nu_ref is done by the caller.
 *
 */
class MHO_DelayRate: public MHO_BinaryOperator< visibility_type, weight_type, sbd_type >
{
    public:
        MHO_DelayRate();
        virtual ~MHO_DelayRate();

        /**
         * @brief Setter for reference frequency
         *
         * @param ref_freq New reference frequency value in MHz
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
         * @brief Executes MHO_DelayRate operations: zero padding, weighting, FFT, and resampling.
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

        //One resampling instruction: output rate bin k of channel c is the linear interpolation out:
        //(1-frac)*spectrum[lo] + frac*spectrum[hi], the interp parameters are precomputed per (ch,dr) so the hot loop
        //carries no fmod or index arithmetic.
        struct InterpEntry
        {
                int lo, hi;
                double frac;
        };

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
         * @brief Builds fInterpTable, the per-(channel, rate-bin) resampling instructions
         *        that put every channel onto a common delay-rate grid. See the class
         *        comment for why this rescaling is needed.
         *
         * @param in1 Input data array of type XArgType1 (supplies the channel sky frequencies)
         */
        void BuildResamplingTable(const XArgType1* in1);

        /**
         * @brief Resamples each channel onto the common delay-rate grid and writes the
         *        rate axis labels.
         *        Loop order is dr(outer)->sbd(inner) so the innermost loop walks contiguous
         *        memory via raw pointers, paying the OffsetFromStrideIndex cost once per
         *        (pp,ch,dr) triple rather than once per element. Results are staged in
         *        fInterpWorkspace to avoid aliasing with the source rows.
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

        MHO_EndZeroPadderOptimized< visibility_type > fZeroPadder;
        FFT_ENGINE_TYPE fFFTEngine;

        int fDRSPSize;
        double fRefFreq;

        bool fInitialized;

        //Resampling instructions, indexed [ch*fDRSPSize + dr]. The lo/hi indices already
        //include the half-length shift that centres zero rate, so they address the raw
        //post-FFT array directly and no separate fftshift pass is needed.
        std::vector< InterpEntry > fInterpTable;

        //staging buffer for ApplyInterpolation: fDRSPSize rows x nsbd columns
        std::vector< sbd_type::value_type > fInterpWorkspace;
};

} // namespace hops

#endif /*! end of include guard: MHO_DelayRate */
