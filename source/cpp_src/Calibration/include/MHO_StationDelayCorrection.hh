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
 *@brief operator to apply a station delay to the visibilities
 */

/**
 * @brief Class MHO_StationDelayCorrection
 */
class MHO_StationDelayCorrection: public MHO_UnaryOperator< visibility_type >
{
    public:
        MHO_StationDelayCorrection();
        virtual ~MHO_StationDelayCorrection();

        /**
         * @brief Setter for reference frequency
         * 
         * @param ref_freq New reference frequency value in MHz
         */
        void SetReferenceFrequency(double ref_freq) { fRefFreq = ref_freq; }

        /**
         * @brief Setter for station identifier
         * 
         * @param station_id mk4 id of type std::string
         * @details station_id is treated as follows:
         * 1-char => mk4 id
         * 2-char => 2char station code
         */
        void SetStationIdentifier(std::string station_id) { fStationIdentity = station_id; }

        /**
         * @brief Setter for delay offset
         * 
         * @param pc_delay_offset Time offset between two signals being correlated
         */
        void SetPCDelayOffset(const double& pc_delay_offset) { fDelayOffset = pc_delay_offset; }

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
         * @brief Initializes out-of-place visibility data from input data.
         * 
         * @param in Const pointer to input visibility_type data.
         * @param out (visibility_type*)
         * @return Boolean indicating successful initialization.
         * @note This is a virtual function.
         */
        virtual bool InitializeOutOfPlace(const visibility_type* in, visibility_type* out) override;

        /**
         * @brief Applies phase correction to visibility data for reference or remote station.
         * 
         * @param in Input visibility_type* containing pol-products and channels.
         * @return bool indicating successful execution.
         * @note This is a virtual function.
         */
        virtual bool ExecuteInPlace(visibility_type* in) override;
        /**
         * @brief Copies input visibility data and executes in-place correction.
         * 
         * @param in Input visibility_type data to be copied.
         * @param out (visibility_type*)
         * @return Result of ExecuteInPlace operation on out parameter.
         * @note This is a virtual function.
         */
        virtual bool ExecuteOutOfPlace(const visibility_type* in, visibility_type* out) override;

    private:
        /**
         * @brief Checks if a station delay correction is applicable based on input parameters.
         * 
         * @param st_idx Index of the station
         * @param in Visibility type pointer containing station data
         * @return Boolean indicating whether to apply the correction or not
         */
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
