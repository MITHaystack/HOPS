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
*@brief Tue Apr  2 09:41:24 AM EDT 2024
*/


class MHO_Passband: public MHO_UnaryOperator< visibility_type >
{
    public:

        MHO_Passband();
        virtual ~MHO_Passband();

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
                //s,o everything outside of this range is cut
                fIsExclusion = false;
                fLow = first;
                fHigh = second;
            }

        }

    protected:

        virtual bool InitializeInPlace(visibility_type* in) override;
        virtual bool InitializeOutOfPlace(const visibility_type* in, visibility_type* out) override;

        virtual bool ExecuteInPlace(visibility_type* in) override;
        virtual bool ExecuteOutOfPlace(const visibility_type* in, visibility_type* out) override;

    private:

        void DetermineChannelFrequencyLimits(double sky_freq, double bandwidth, std::string net_sideband, double& lower_freq, double& upper_freq);

        bool fIsExclusion;
        double fLow;
        double fHigh;

        std::string fBandwidthKey;
        std::string fSidebandLabelKey;
        std::string fLowerSideband;
        std::string fUpperSideband;


};


}


#endif /*! end of include guard: MHO_Passband */
