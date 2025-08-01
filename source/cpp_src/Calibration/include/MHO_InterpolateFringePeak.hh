#ifndef MHO_InterpolateFringePeak_HH__
#define MHO_InterpolateFringePeak_HH__

#include <cmath>
#include <complex>

#include "MHO_ContainerDefinitions.hh"
#include "MHO_Operator.hh"

#include "MHO_FringeRotation.hh"

namespace hops
{

/*!
 *@file MHO_InterpolateFringePeak.hh
 *@class MHO_InterpolateFringePeak
 *@author J. Barrettj - barrettj@mit.edu
 *@date Thu Apr 13 16:25:38 2023 -0400
 *@brief implements fine interpolation about the fringe peak (see interp.c and max555.c code)
 */

/**
 * @brief Class MHO_InterpolateFringePeak
 */
class MHO_InterpolateFringePeak: public MHO_Operator
{
    public:
        MHO_InterpolateFringePeak();
        virtual ~MHO_InterpolateFringePeak(){};

        /**
         * @brief Sets optimize closure to true (not used for 'simul' method).
         */
        void EnableOptimizeClosure() { fRot.SetOptimizeClosureTrue(); }

        /**
         * @brief Disables optimize closure
         */
        void DisableOptimizeClosure() { fRot.SetOptimizeClosureFalse(); }

        /**
         * @brief Setter for reference frequency
         *
         * @param ref_freq New reference frequency value in Hertz
         */
        void SetReferenceFrequency(double ref_freq) { fRefFreq = ref_freq; }

        /**
         * @brief Setter for reference time offset
         *
         * @param frt_offset New reference time offset value
         */
        void SetReferenceTimeOffset(double frt_offset) { fFRTOffset = frt_offset; }

        /**
         * @brief Setter for max bins (location)
         *
         * @param sbd_max Set the bin location for the maximum along the Single-Band Delay (SBD) axis .
         * @param mbd_max Set the bin location for the maximum along the Multi-Band Delay (MBD) axis.
         * @param dr_max Set the bin location for the maximum along the Delay Rate (DR) axis.
         */
        void SetMaxBins(int sbd_max, int mbd_max, int dr_max);

        /**
         * @brief Setter for sbd array
         *
         * @param sbd_arr Input SBD array of type const visibility_type*
         */
        void SetSBDArray(const visibility_type* sbd_arr) { fSBDArray = sbd_arr; }

        /**
         * @brief Setter for weights
         *
         * @param weights Input weights of type const weight_type*
         */
        void SetWeights(const weight_type* weights) { fWeights = weights; }

        /**
         * @brief Setter for mbd axis
         *
         * @param mbd_ax Input Multi-Band Delay axis
         */
        void SetMBDAxis(const time_axis_type* mbd_ax) { fMBDAxis.Copy(*mbd_ax); }

        /**
         * @brief Setter for dr axis
         *
         * @param dr_ax Input delay_rate_axis_type data to copy
         */
        void SetDRAxis(const delay_rate_axis_type* dr_ax) { fDRAxis.Copy(*dr_ax); }

        /**
         * @brief Initializes MHO_InterpolateFringePeak object by checking and retrieving necessary data.
         *
         * @return True if initialization is successful, false otherwise.
         * @note This is a virtual function.
         */
        virtual bool Initialize() override;
        /**
         * @brief Executes fine peak interpolation and returns success.
         *
         * @return True if execution is successful.
         * @note This is a virtual function.
         */
        virtual bool Execute() override;

        /**
         * @brief Getter for sbdelay
         *
         * @return Current value of sbdelay as a double
         */
        double GetSBDelay() const { return fSBDelay; }

        /**
         * @brief Getter for mbdelay
         *
         * @return Current multi-band delay as a double.
         */
        double GetMBDelay() const { return fMBDelay; }

        /**
         * @brief Getter for delay rate
         *
         * @return The current delay rate as a double.
         */
        double GetDelayRate() const { return fDelayRate; }

        /**
         * @brief Getter for fringe rate
         *
         * @return The current fringe rate as a double.
         */
        double GetFringeRate() const { return fFringeRate; }

        /**
         * @brief Getter for fringe amplitude
         *
         * @return Current fringe amplitude as a double.
         */
        double GetFringeAmplitude() const { return fFringeAmp; }

    private:
        int fMBDMaxBin;
        int fDRMaxBin;
        int fSBDMaxBin;

        double fRefFreq;
        double fFRTOffset;
        double fTotalSummedWeights;
        const visibility_type* fSBDArray;
        const weight_type* fWeights;

        time_axis_type fMBDAxis;
        delay_rate_axis_type fDRAxis;

        void fine_peak_interpolation();

        MHO_NDArrayWrapper< double, 3 > fDRF;

        double fSBDelay;
        double fMBDelay;
        double fDelayRate;
        double fFringeRate;
        double fFringeAmp;

        //copy of max555.c impl
        void max555(MHO_NDArrayWrapper< double, 3 >&, double xlim[3][2], double xi[3], double* drfmax);
        void interp555(MHO_NDArrayWrapper< double, 3 >&, double xi[3], double* drfval);
        double dwin(double, double, double);

        //class which implements vrot
        MHO_FringeRotation fRot;
};

} // namespace hops

#endif /*! end of include guard: MHO_InterpolateFringePeak_HH__ */
