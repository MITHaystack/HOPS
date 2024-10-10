#ifndef MHO_PhaseCalibrationTrim_HH__
#define MHO_PhaseCalibrationTrim_HH__

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
 *@file MHO_PhaseCalibrationTrim.hh
 *@class MHO_PhaseCalibrationTrim
 *@author J. Barrett - barrettj@mit.edu
 *@date Thu Jan 27 10:36:00 2022 -0500
 *@brief Trims the time range of the pcal data to match that of the visibilities, very minimal implementation. 
 *Assumes that the p-cal and visibility data have the same accumulation period (this is true of most sane data)
 *If they did not, then we would need to interpolate between p-cal points to match the visbility gridding (this is not done here)
 */

class MHO_PhaseCalibrationTrim: public MHO_UnaryOperator< multitone_pcal_type >
{
    public:
        MHO_PhaseCalibrationTrim();
        virtual ~MHO_PhaseCalibrationTrim();

        void SetVisibilities(const visibility_type* vis){fVis = vis;}

    protected:
        virtual bool InitializeInPlace(multitone_pcal_type* in) override;
        virtual bool InitializeOutOfPlace(const multitone_pcal_type* in, multitone_pcal_type* out) override;

        virtual bool ExecuteInPlace(multitone_pcal_type* in) override;
        virtual bool ExecuteOutOfPlace(const multitone_pcal_type* in, multitone_pcal_type* out) override;

    private:

        const visibility_type* fVis;

};

} // namespace hops

#endif /*! end of include guard: MHO_PhaseCalibrationTrim */
