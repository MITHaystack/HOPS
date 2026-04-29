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

/**
 * @brief Class MHO_IonosphericPhaseCorrection
 */
class MHO_IonosphericPhaseCorrection: public MHO_UnaryOperator< visibility_type >
{
    public:
        MHO_IonosphericPhaseCorrection();
        virtual ~MHO_IonosphericPhaseCorrection();

        /**
         * @brief Setter for differential TEC (controls phase dispersion)
         *
         * @param dTEC Input differential TEC value
         */
        void SetDifferentialTEC(double dTEC) { fdTEC = dTEC; };

    protected:
        /**
         * @brief Applies differential ionospheric phase correction to visibility data in-place.
         *
         * @param in Input visibility_type* containing pol-products and channels.
         * @return bool indicating successful execution.
         * @note This is a virtual function.
         */
        virtual bool ExecuteInPlace(visibility_type* in) override;

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
