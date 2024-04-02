#ifndef MHO_Passband_HH__
#define MHO_Passband_HH__


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
*@file MHO_Passband.hh
*@class MHO_Passband
*@author J. Barrett - barrettj@mit.edu
*@date
*@briefe Tue Apr  2 09:41:24 AM EDT 2024
*/


class MHO_Passband: public MHO_UnaryOperator< visibility_type >
{
    public:

        MHO_Passband();
        virtual ~MHO_Passband();

        void SetPassband(const double& low, const double& high)
        {
            fLow = low;
            fHigh = high;
        }

    protected:

        virtual bool InitializeInPlace(visibility_type* in) override;
        virtual bool InitializeOutOfPlace(const visibility_type* in, visibility_type* out) override;

        virtual bool ExecuteInPlace(visibility_type* in) override;
        virtual bool ExecuteOutOfPlace(const visibility_type* in, visibility_type* out) override;

    private:

        double fLow;
        double fHigh;


};


}


#endif /*! end of include guard: MHO_Passband */
