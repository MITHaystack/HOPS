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

/**
 * @brief Class MHO_PhaseCalibrationTrim
 */
class MHO_PhaseCalibrationTrim: public MHO_UnaryOperator< multitone_pcal_type >
{
    public:
        MHO_PhaseCalibrationTrim();
        virtual ~MHO_PhaseCalibrationTrim();

        /**
         * @brief Setter for visibilities
         * 
         * @param vis Input const visibility_type* array containing visibilities
         */
        void SetVisibilities(const visibility_type* vis) { fVis = vis; }

    protected:
        /**
         * @brief Trims down multitone_pcal_type data for phase calibration in-place.
         * 
         * @param in Input pointer to multitone_pcal_type data.
         * @return True if trimming was successful.
         * @note This is a virtual function.
         */
        virtual bool InitializeInPlace(multitone_pcal_type* in) override;
        /**
         * @brief Initializes out-of-place phase calibration trim using input and output multitone_pcal_type.
         * 
         * @param in Const reference to input multitone_pcal_type
         * @param out Reference to output multitone_pcal_type
         * @return Boolean indicating successful initialization
         * @note This is a virtual function.
         */
        virtual bool InitializeOutOfPlace(const multitone_pcal_type* in, multitone_pcal_type* out) override;

        /**
         * @brief Trims and aligns multitone phase calibration data in-place based on visibility data.
         * 
         * @param in Input multitone_pcal_type* containing phase calibration data to be trimmed
         * @return bool indicating success or failure of the trimming operation
         * @note This is a virtual function.
         */
        virtual bool ExecuteInPlace(multitone_pcal_type* in) override;
        /**
         * @brief Copies input multitone_pcal_type and executes in-place trim.
         * 
         * @param in Input multitone_pcal_type data to be copied
         * @param out (multitone_pcal_type*)
         * @return Result of ExecuteInPlace operation on copied data
         * @note This is a virtual function.
         */
        virtual bool ExecuteOutOfPlace(const multitone_pcal_type* in, multitone_pcal_type* out) override;

    private:
        const visibility_type* fVis;
        double fAPEps;
        double fStartEps;
};

} // namespace hops

#endif /*! end of include guard: MHO_PhaseCalibrationTrim */
