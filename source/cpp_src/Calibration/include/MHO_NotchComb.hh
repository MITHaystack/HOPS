#ifndef MHO_NotchComb_HH__
#define MHO_NotchComb_HH__

#include <cctype>
#include <cmath>
#include <complex>
#include <map>
#include <utility>
#include <vector>

#include "MHO_Constants.hh"
#include "MHO_MathUtilities.hh"
#include "MHO_Message.hh"

#include "MHO_ContainerDefinitions.hh"
#include "MHO_TableContainer.hh"
#include "MHO_UnaryOperator.hh"

namespace hops
{

/*!
 *@file MHO_NotchComb.hh
 *@class MHO_NotchComb
 *@author J. Barrett - barrettj@mit.edu
 *@date Tue Apr  2 09:41:24 AM EDT 2024
 *@brief operator which 'notches' out problematic chunks of visiblities in frequency space
 */

/**
 * @brief Class MHO_NotchComb
 */
class MHO_NotchComb: public MHO_UnaryOperator< visibility_type >
{
    public:
        MHO_NotchComb();
        virtual ~MHO_NotchComb();

        /**
         * @brief Setter for weights
         *
         * @param weights Input weight values of type weight_type
         */
        void SetWeights(weight_type* weights) { fWeights = weights; };

        //starting frequency offset (assumed to be zero by default)
        void SetNotchOffset(double offset) { fNotchOffset = offset; }

        double GetNotchOffset() const { return fNotchOffset; }

        //spacing between the notches
        void SetNotchPeriod(double period) { fNotchPeriod = period; }

        double GetNotchPeriod() const { return fNotchPeriod; }

        //notch width must be less than the notch period!
        void SetNotchWidth(double width) { fNotchWidth = width; }

        double GetNotchWidth() const { return fNotchWidth; }

    protected:
        /**
         * @brief Applies filter to channels and spectral points based on defined notches.
         *
         * @param in Input visibility data containing channel axis and frequency axis.
         * @return True if execution is successful, false otherwise.
         * @note This is a virtual function.
         */
        virtual bool ExecuteInPlace(visibility_type* in) override;

    private:
        std::string fBandwidthKey;
        std::string fSidebandLabelKey;
        std::string fLowerSideband;
        std::string fUpperSideband;

        weight_type* fWeights;

        double fNotchOffset; //MHz
        double fNotchPeriod; //MHz
        double fNotchWidth;  //MHz
};

} // namespace hops

#endif /*! end of include guard: MHO_NotchComb */
