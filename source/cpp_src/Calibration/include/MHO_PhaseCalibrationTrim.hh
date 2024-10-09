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
 *@brief
 */

class MHO_PhaseCalibrationTrim: public MHO_UnaryOperator< visibility_type >
{
    public:
        MHO_PhaseCalibrationTrim();
        virtual ~MHO_PhaseCalibrationTrim();

        

    protected:
        virtual bool InitializeInPlace(visibility_type* in) override;
        virtual bool InitializeOutOfPlace(const visibility_type* in, visibility_type* out) override;

        virtual bool ExecuteInPlace(visibility_type* in) override;
        virtual bool ExecuteOutOfPlace(const visibility_type* in, visibility_type* out) override;

    private:

};

} // namespace hops

#endif /*! end of include guard: MHO_PhaseCalibrationTrim */
