#ifndef MHO_StationDelayCorrection_HH__
#define MHO_StationDelayCorrection_HH__

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
 *@file MHO_StationDelayCorrection.hh
 *@class MHO_StationDelayCorrection
 *@author J. Barrett - barrettj@mit.edu
 *@date Thu Jan 27 10:36:00 2022 -0500
 *@brief
 */

class MHO_StationDelayCorrection: public MHO_UnaryOperator< visibility_type >
{
    public:
        MHO_StationDelayCorrection();
        virtual ~MHO_StationDelayCorrection();

        void SetReferenceFrequency(double ref_freq) { fRefFreq = ref_freq; }

        //treated as follows:
        //1-char => mk4 id
        //2-char => 2char station code
        void SetStationIdentifier(std::string station_id) { fStationIdentity = station_id; }

        void SetPCDelayOffset(const double& pc_delay_offset) { fDelayOffset = pc_delay_offset; }

    protected:
        virtual bool InitializeInPlace(visibility_type* in) override;
        virtual bool InitializeOutOfPlace(const visibility_type* in, visibility_type* out) override;

        virtual bool ExecuteInPlace(visibility_type* in) override;
        virtual bool ExecuteOutOfPlace(const visibility_type* in, visibility_type* out) override;

    private:
        bool IsApplicable(std::size_t st_idx, const visibility_type* in);

        //constants
        std::complex< double > fImagUnit;
        double fNanoSecToSecond;
        double fMHzToHz;
        double fPi;

        //selection
        std::string fStationIdentity;

        //ref freq and pc delay
        double fRefFreq;
        double fDelayOffset;

        //keys for tag retrieval
        std::string fStationKey;
        std::string fRemStationKey;
        std::string fRefStationKey;
        std::string fRemStationMk4IDKey;
        std::string fRefStationMk4IDKey;

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

#endif /*! end of include guard: MHO_StationDelayCorrection */
