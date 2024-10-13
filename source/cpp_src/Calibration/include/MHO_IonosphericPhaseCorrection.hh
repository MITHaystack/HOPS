#ifndef MHO_IonosphericPhaseCorrection_HH__
#define MHO_IonosphericPhaseCorrection_HH__

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
 *@file MHO_IonosphericPhaseCorrection.hh
 *@class MHO_IonosphericPhaseCorrection
 *@author J. Barrett - barrettj@mit.edu
 *@date Wed Jan 17 15:31:52 2024 -0500
 *@brief
 */

class MHO_IonosphericPhaseCorrection: public MHO_UnaryOperator< visibility_type >
{
    public:
        MHO_IonosphericPhaseCorrection();
        virtual ~MHO_IonosphericPhaseCorrection();

        void SetDifferentialTEC(double dTEC) { fdTEC = dTEC; };

    protected:
        virtual bool InitializeInPlace(visibility_type* in) override;
        virtual bool InitializeOutOfPlace(const visibility_type* in, visibility_type* out) override;

        virtual bool ExecuteInPlace(visibility_type* in) override;
        virtual bool ExecuteOutOfPlace(const visibility_type* in, visibility_type* out) override;

    private:
        double fdTEC;

        //constants
        double fIonoK; //dTEC to phase constant
        std::complex< double > fImagUnit;
        double fDegToRad;

        //keys for tag retrieval and matching
        std::string fChannelLabelKey;
        std::string fSidebandLabelKey;
        std::string fBandwidthKey;
        std::string fLowerSideband;
        std::string fUpperSideband;

        //minor helper function to make sure all strings are compared as upper-case only
        void make_upper(std::string& s)
        {
            for(char& c : s)
            {
                c = toupper(c);
            };
        }
};

} // namespace hops

#endif /*! end of include guard: MHO_IonosphericPhaseCorrection */
