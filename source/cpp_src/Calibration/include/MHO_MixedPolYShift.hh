#ifndef MHO_MixedPolYShift_HH__
#define MHO_MixedPolYShift_HH__

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
 *@file MHO_MixedPolYShift.hh
 *@class MHO_MixedPolYShift
 *@author J. Barrett - barrettj@mit.edu
 *@date Fri Oct 11 01:53:13 PM EDT 2024
 *@brief
 */

class MHO_MixedPolYShift: public MHO_UnaryOperator< visibility_type >
{
    public:
        MHO_MixedPolYShift();
        virtual ~MHO_MixedPolYShift();

        //not called by the builder (yet)...
        //this particular operator feature always uses 90 degrees
        //but perhaps we could allow that to change via this setter
        void SetPhaseOffset(const double& offset) { fYPolPhaseOffset = offset; }

    protected:
        virtual bool InitializeInPlace(visibility_type* in) override;
        virtual bool InitializeOutOfPlace(const visibility_type* in, visibility_type* out) override;

        virtual bool ExecuteInPlace(visibility_type* in) override;
        virtual bool ExecuteOutOfPlace(const visibility_type* in, visibility_type* out) override;

    private:
        bool IsApplicable(std::size_t st_idx, std::string polprod);
        bool PolMatch(std::size_t station_idx, std::string& polprod);

        //constants
        std::complex< double > fImagUnit;
        double fDegToRad;

        //selection
        std::string fStationIdentity;

        //phase rotation (90 degrees)
        double fYPolPhaseOffset;

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

        //determines if this is a pol-product of the form (YR, XR, XL, YL, LX, etc.)
        bool IsMixedLinCirc(std::string polprod) const;

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

#endif /*! end of include guard: MHO_MixedPolYShift */
