#ifndef MHO_Passband_HH__
#define MHO_Passband_HH__

#include <cctype>
#include <cmath>
#include <complex>
#include <map>
#include <vector>

#include "MHO_Constants.hh"
#include "MHO_Message.hh"

#include "MHO_ContainerDefinitions.hh"
#include "MHO_TableContainer.hh"
#include "MHO_UnaryOperator.hh"

namespace hops
{

/*!
 *@file MHO_Passband.hh
 *@class MHO_Passband
 *@author J. Barrett - barrettj@mit.edu
 *@date Tue Apr  2 09:41:24 AM EDT 2024
 *@brief Selects a chunk of frequency space for inclusion or removal
 */

/**
 * @brief Class MHO_Passband
 */
class MHO_Passband: public MHO_UnaryOperator< visibility_type >
{
    public:
        MHO_Passband();
        virtual ~MHO_Passband();

        /**
         * @brief Setter for weights
         *
         * @param weights Input weight values of type weight_type
         */
        void SetWeights(weight_type* weights) { fWeights = weights; };

        /**
         * @brief Setter for passband
         *
         * @param first Lower frequency limit in Hz
         * @param second Upper frequency limit in Hz
         * @details  if second < first then this operation is an 'exclusion'
         * which tells us this is a chunk of spectrum to cut out, otherwise
         * if first < second, then it is an inclusion, and everything outside will be cut
         * (this is the legacy behavior)
         */
        void SetPassband(const double& first, const double& second)
        {
            //if second > first then this is an 'exclusion'
            //telling us this is a chunk of spectrum to cut
            //this is the legacy behavior
            if(second < first)
            {
                fIsExclusion = true;
                fLow = second;
                fHigh = first;
            }
            else
            {
                //if first < second, then this chunk is the 'passed' band
                //so everything outside of this range is cut
                fIsExclusion = false;
                fLow = first;
                fHigh = second;
            }
        }

    protected:
        /**
         * @brief Initializes visibility_type pointer in-place and returns true.
         *
         * @param in Pointer to visibility_type that will be initialized in-place.
         * @return Boolean indicating success (always true).
         * @note This is a virtual function.
         */
        virtual bool InitializeInPlace(visibility_type* in) override;
        /**
         * @brief Initializes out-of-place visibility data.
         *
         * @param in Input visibility_type pointer
         * @param out Output visibility_type pointer
         * @return True if initialization is successful.
         * @note This is a virtual function.
         */
        virtual bool InitializeOutOfPlace(const visibility_type* in, visibility_type* out) override;

        /**
         * @brief Checks and excludes specific passbands within each channel of visibility data.
         *
         * @param in Input visibility_type* containing channel axis.
         * @return bool indicating successful exclusion operation.
         * @note This is a virtual function.
         */
        virtual bool ExecuteInPlace(visibility_type* in) override;
        /**
         * @brief Copies input visibility data and executes in-place processing.
         *
         * @param in Input visibility_type data to be copied.
         * @param out (visibility_type*)
         * @return Result of ExecuteInPlace operation on out parameter.
         * @note This is a virtual function.
         */
        virtual bool ExecuteOutOfPlace(const visibility_type* in, visibility_type* out) override;

    private:
        bool fIsExclusion;
        double fLow;
        double fHigh;

        std::string fBandwidthKey;
        std::string fSidebandLabelKey;
        std::string fLowerSideband;
        std::string fUpperSideband;

        weight_type* fWeights;
};

} // namespace hops

#endif /*! end of include guard: MHO_Passband */
