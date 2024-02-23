#ifndef MHO_IonosphericPhaseCorrection_HH__
#define MHO_IonosphericPhaseCorrection_HH__

/*!
*@file MHO_IonosphericPhaseCorrection.hh
*@class MHO_IonosphericPhaseCorrection
*@author J. Barrett - barrettj@mit.edu
*@date
*@brief
*/

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


class MHO_IonosphericPhaseCorrection: public MHO_UnaryOperator< visibility_type >
{
    public:

        MHO_IonosphericPhaseCorrection();
        virtual ~MHO_IonosphericPhaseCorrection();

        void SetDifferentialTEC(double dTEC){fdTEC = dTEC;};

    protected:

        virtual bool InitializeInPlace(visibility_type* in) override;
        virtual bool InitializeOutOfPlace(const visibility_type* in, visibility_type* out) override;

        virtual bool ExecuteInPlace(visibility_type* in) override;
        virtual bool ExecuteOutOfPlace(const visibility_type* in, visibility_type* out) override;

    private:


        double fdTEC;

        //constants
        double fIonoK; //dTEC to phase constant
        std::complex<double> fImagUnit;
        double fDegToRad;

        //keys for tag retrieval and matching
        std::string fChannelLabelKey;
        std::string fSidebandLabelKey;
        std::string fBandwidthKey;
        std::string fLowerSideband;
        std::string fUpperSideband;

        //figures out upper/lower frequency bounds of each channel
        void DetermineChannelFrequencyLimits(double sky_freq, double bandwidth, std::string net_sideband, double& lower_freq, double& upper_freq);

        //minor helper function to make sure all strings are compared as upper-case only
        void make_upper(std::string& s){ for(char& c : s){c = toupper(c); };
    }


};


}


#endif /*! end of include guard: MHO_IonosphericPhaseCorrection */
