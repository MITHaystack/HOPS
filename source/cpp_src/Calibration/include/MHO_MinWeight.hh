#ifndef MHO_MinWeight_HH__
#define MHO_MinWeight_HH__


#include <cmath>
#include <complex>
#include <vector>
#include <map>
#include <cctype>

#include "MHO_Message.hh"
#include "MHO_Constants.hh"

#include "MHO_TableContainer.hh"
#include "MHO_ContainerDefinitions.hh"
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


class MHO_MinWeight: public MHO_UnaryOperator< weight_type >
{
    public:

        MHO_MinWeight();
        virtual ~MHO_MinWeight();

        void SetMinWeight(double min_weight){fMinWeight = min_weight;}

    protected:

        virtual bool InitializeInPlace(weight_type* in) override;
        virtual bool InitializeOutOfPlace(const weight_type* in, weight_type* out) override;

        virtual bool ExecuteInPlace(weight_type* in) override;
        virtual bool ExecuteOutOfPlace(const weight_type* in, weight_type* out) override;

    private:

        double fMinWeight;

};


}


#endif /*! end of include guard: MHO_MinWeight */
