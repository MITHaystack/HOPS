#ifndef MHO_LSBOffset_HH__
#define MHO_LSBOffset_HH__

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
 *@file MHO_LSBOffset.hh
 *@class MHO_LSBOffset
 *@author J. Barrett - barrettj@mit.edu
 *@date Thu Jan 27 10:36:00 2022 -0500
 *@brief
 */

class MHO_LSBOffset: public MHO_UnaryOperator< visibility_type >
{
    public:
        MHO_LSBOffset();
        virtual ~MHO_LSBOffset();

        //treated as follows:
        //1-char => mk4 id
        //2-char => 2char station code
        void SetStationIdentifier(std::string station_id) { fStationIdentity = station_id; }

        //set lsb (phase) offset for double-sideband channels
        void SetLSBPhaseOffset(const double& lsb_offset) { fLSBPhaseOffset = lsb_offset; }

    protected:
        virtual bool InitializeInPlace(visibility_type* in) override;
        virtual bool InitializeOutOfPlace(const visibility_type* in, visibility_type* out) override;

        virtual bool ExecuteInPlace(visibility_type* in) override;
        virtual bool ExecuteOutOfPlace(const visibility_type* in, visibility_type* out) override;

    private:
        bool IsApplicable(std::size_t st_idx, const visibility_type* in);
        bool PolMatch(std::size_t station_idx, std::string& polprod);

        //constants
        std::complex< double > fImagUnit;
        double fDegToRad;

        //selection
        std::string fStationIdentity;

        //pc rotation
        double fLSBPhaseOffset;

        //keys for tag retrieval
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

#endif /*! end of include guard: MHO_LSBOffset */
