#ifndef MHO_MinWeight_HH__
#define MHO_MinWeight_HH__

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
 *@file MHO_MinWeight.hh
 *@class MHO_MinWeight
 *@author J. Barrett - barrettj@mit.edu
 *@date
 *@brief Tue Apr  2 09:41:24 AM EDT 2024
 */

/**
 * @brief Class MHO_MinWeight
 */
class MHO_MinWeight: public MHO_UnaryOperator< weight_type >
{
    public:
        MHO_MinWeight();
        virtual ~MHO_MinWeight();

        /**
         * @brief Setter for min weight
         *
         * @param min_weight the minimum allowed weight value
         */
        void SetMinWeight(double min_weight) { fMinWeight = min_weight; }

    protected:

        /**
         * @brief Zeroes out weights in-place for all values less than a specified minimum.
         *
         * @param in Input weight array to be modified in-place.
         * @return True if execution was successful.
         * @note This is a virtual function.
         */
        virtual bool ExecuteInPlace(weight_type* in) override;

    private:
        double fMinWeight;
};

} // namespace hops

#endif /*! end of include guard: MHO_MinWeight */
