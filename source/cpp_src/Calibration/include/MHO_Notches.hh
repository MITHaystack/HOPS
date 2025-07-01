#ifndef MHO_Notches_HH__
#define MHO_Notches_HH__

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
 *@file MHO_Notches.hh
 *@class MHO_Notches
 *@author J. Barrett - barrettj@mit.edu
 *@date
 *@brief Tue Apr  2 09:41:24 AM EDT 2024
 */

/**
 * @brief Class MHO_Notches
 */
class MHO_Notches: public MHO_UnaryOperator< visibility_type >
{
    public:
        MHO_Notches();
        virtual ~MHO_Notches();

        /**
         * @brief Setter for weights
         * 
         * @param weights Input weight values of type weight_type
         */
        void SetWeights(weight_type* weights) { fWeights = weights; };

        /**
         * @brief Setter for notch boundaries
         * 
         * @param notch_boundary_list Input vector of double values representing notch boundaries.
         */
        void SetNotchBoundaries(const std::vector< double >& notch_boundary_list)
        {
            fNotchBoundaries = notch_boundary_list;
            std::size_t N = fNotchBoundaries.size();
            if(N % 2 != 0)
            {
                msg_warn("calibration", "the number of notch boundaries passed ("
                                            << N << ") is not a multiple of 2, malformed control input " << eom);
                N = N - 1; //drop the last value, in effort to salvage it
            }

            //loop over the notch boundaries pairing them up
            for(std::size_t i = 0; i < N;)
            {
                double low = fNotchBoundaries[i];
                double high = fNotchBoundaries[i + 1];
                fNotches.push_back(std::make_pair(low, high));
                i += 2;
            }
        }

    protected:
        /**
         * @brief Initializes MHO_Notches in-place using provided visibility_type pointer.
         * 
         * @param in Pointer to visibility_type for initialization.
         * @return True if initialization is successful.
         * @note This is a virtual function.
         */
        virtual bool InitializeInPlace(visibility_type* in) override;
        /**
         * @brief Initializes out-of-place visibility data from input pointer.
         * 
         * @param in Const pointer to input visibility_type data.
         * @param out (visibility_type*)
         * @return Boolean indicating successful initialization.
         * @note This is a virtual function.
         */
        virtual bool InitializeOutOfPlace(const visibility_type* in, visibility_type* out) override;

        /**
         * @brief Applies filter to channels and spectral points based on defined notches.
         * 
         * @param in Input visibility data containing channel axis and frequency axis.
         * @return True if execution is successful, false otherwise.
         * @note This is a virtual function.
         */
        virtual bool ExecuteInPlace(visibility_type* in) override;
        /**
         * @brief Copies input visibility data and executes in-place processing.
         * 
         * @param in Const reference to input visibility_type data.
         * @param out (visibility_type*)
         * @return Boolean result of ExecuteInPlace operation on copied output data.
         * @note This is a virtual function.
         */
        virtual bool ExecuteOutOfPlace(const visibility_type* in, visibility_type* out) override;

    private:
        std::string fBandwidthKey;
        std::string fSidebandLabelKey;
        std::string fLowerSideband;
        std::string fUpperSideband;

        weight_type* fWeights;

        std::vector< double > fNotchBoundaries;
        std::vector< std::pair< double, double > > fNotches;
};

} // namespace hops

#endif /*! end of include guard: MHO_Notches */
