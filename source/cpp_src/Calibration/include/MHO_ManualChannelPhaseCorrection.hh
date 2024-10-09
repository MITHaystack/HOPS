#ifndef MHO_ManualChannelPhaseCorrection_HH__
#define MHO_ManualChannelPhaseCorrection_HH__

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
 *@file MHO_ManualChannelPhaseCorrection.hh
 *@class MHO_ManualChannelPhaseCorrection
 *@author J. Barrett - barrettj@mit.edu
 *@date Thu Jan 27 10:36:00 2022 -0500
 *@brief
 */

class MHO_ManualChannelPhaseCorrection: public MHO_UnaryOperator< visibility_type >
{
    public:
        MHO_ManualChannelPhaseCorrection();
        virtual ~MHO_ManualChannelPhaseCorrection();

        //treated as follows:
        //1-char => mk4 id
        //2-char => 2char station code
        void SetStationIdentifier(std::string station_id) { fStationIdentity = station_id; }

        void SetPolarization(const std::string& pol)
        {
            fPol = pol;
            make_upper(fPol);
        };

        //channel label -> pc_phases
        void SetChannelToPCPhaseMap(const std::map< std::string, double >& map) { fPCMap = map; };

    protected:
        virtual bool InitializeInPlace(visibility_type* in) override;
        virtual bool InitializeOutOfPlace(const visibility_type* in, visibility_type* out) override;

        virtual bool ExecuteInPlace(visibility_type* in) override;
        virtual bool ExecuteOutOfPlace(const visibility_type* in, visibility_type* out) override;

    private:
        bool IsApplicable(std::size_t st_idx, const visibility_type* in);
        bool PolMatch(std::size_t station_idx, std::string& polprod);

        bool LabelMatch(std::string expected_chan_label, std::string given_chan_label);

        //constants
        std::complex< double > fImagUnit;
        double fDegToRad;

        //selection
        std::string fStationIdentity;
        std::string fPol;

        //channel label -> pcal phases
        std::map< std::string, double > fPCMap;

        //keys for tag retrieval and matching
        std::string fStationKey;
        std::string fRemStationKey;
        std::string fRefStationKey;
        std::string fRemStationMk4IDKey;
        std::string fRefStationMk4IDKey;
        std::string fChannelLabelKey;
        std::string fSidebandLabelKey;
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

#endif /*! end of include guard: MHO_ManualChannelPhaseCorrection */
