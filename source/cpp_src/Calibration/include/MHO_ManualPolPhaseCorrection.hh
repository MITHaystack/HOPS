#ifndef MHO_ManualPolPhaseCorrection_HH__
#define MHO_ManualPolPhaseCorrection_HH__

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
 *@file MHO_ManualPolPhaseCorrection.hh
 *@class MHO_ManualPolPhaseCorrection
 *@author J. Barrett - barrettj@mit.edu
 *@date Thu Jan 27 10:36:00 2022 -0500
 *@brief
 */

/**
 * @brief Class MHO_ManualPolPhaseCorrection
 */
class MHO_ManualPolPhaseCorrection: public MHO_UnaryOperator< visibility_type >
{
    public:
        MHO_ManualPolPhaseCorrection();
        virtual ~MHO_ManualPolPhaseCorrection();

        //treated as follows:
        //1-char => mk4 id
        //2-char => 2char station code
        /**
         * @brief Setter for station identifier
         * 
         * @param station_id Two-character MK4 station code as std::string
         */
        void SetStationIdentifier(std::string station_id) { fStationIdentity = station_id; }

        /**
         * @brief Setter for polarization
         * 
         * @param pol Input polarization string
         */
        void SetPolarization(const std::string& pol)
        {
            fPol = pol;
            make_upper(fPol);
        };

        /**
         * @brief Setter for pcphase offset
         * 
         * @param pc_phase_offset New Phase Calibration phase offset value
         */
        void SetPCPhaseOffset(const double& pc_phase_offset) { fPhaseOffset = pc_phase_offset; }

    protected:
        /**
         * @brief Initializes in-place visibility_type pointer.
         * 
         * @param in Input visibility_type pointer to initialize
         * @return True if initialization is successful
         * @note This is a virtual function.
         */
        virtual bool InitializeInPlace(visibility_type* in) override;
        /**
         * @brief Initializes out-of-place visibility data from input pointer.
         * 
         * @param in Const pointer to input visibility_type data
         * @param out (visibility_type*)
         * @return Boolean indicating success (always true)
         * @note This is a virtual function.
         */
        virtual bool InitializeOutOfPlace(const visibility_type* in, visibility_type* out) override;

        /**
         * @brief Applies manual phase correction offsets to visibility data in-place for reference and remote stations.
         * 
         * @param in Input visibility_type* containing pol-products and channels.
         * @return bool indicating successful execution.
         * @note This is a virtual function.
         */
        virtual bool ExecuteInPlace(visibility_type* in) override;
        /**
         * @brief Copies input visibility data and executes in-place correction.
         * 
         * @param in Const reference to input visibility_type data.
         * @param out (visibility_type*)
         * @return Result of ExecuteInPlace operation on copied output data.
         * @note This is a virtual function.
         */
        virtual bool ExecuteOutOfPlace(const visibility_type* in, visibility_type* out) override;

    private:
        /**
         * @brief Checks if manual polarization phase correction is applicable for a given station index and visibility data.
         * 
         * @param st_idx Index of the station (ref/rem).
         * @param in Visibility data to retrieve station information.
         * @return Boolean indicating whether manual correction should be applied.
         */
        bool IsApplicable(std::size_t st_idx, const visibility_type* in);
        /**
         * @brief Checks if polarization product at given station index matches first polarization in fPol array.
         * 
         * @param station_idx Index of the station to check
         * @param polprod (std::string&)
         * @return True if polarization product matches first polarization in fPol, false otherwise
         */
        bool PolMatch(std::size_t station_idx, std::string& polprod);

        //constants
        std::complex< double > fImagUnit;
        double fDegToRad;

        //selection
        std::string fStationIdentity;
        std::string fPol;

        //pc rotation
        double fPhaseOffset;

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

#endif /*! end of include guard: MHO_ManualPolPhaseCorrection */
