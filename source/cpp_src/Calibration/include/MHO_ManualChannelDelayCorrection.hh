#ifndef MHO_ManualChannelDelayCorrection_HH__
#define MHO_ManualChannelDelayCorrection_HH__

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
 *@file MHO_ManualChannelDelayCorrection.hh
 *@class MHO_ManualChannelDelayCorrection
 *@author J. Barrett - barrettj@mit.edu
 *@date Thu Jan 27 10:36:00 2022 -0500
 *@brief
 */

/**
 * @brief Class MHO_ManualChannelDelayCorrection
 */
class MHO_ManualChannelDelayCorrection: public MHO_UnaryOperator< visibility_type >
{
    public:
        MHO_ManualChannelDelayCorrection();
        virtual ~MHO_ManualChannelDelayCorrection();

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
         * @brief Setter for polarization (associated with these delay corrections)
         * 
         * @param pol Input polarization string
         */
        void SetPolarization(const std::string& pol)
        {
            fPol = pol;
            make_upper(fPol);
        };


        /**
         * @brief Setter for channel to pc_delay map
         * 
         * @param map Input map of channel labels and corresponding pc_delays
         */
        void SetChannelToPCDelayMap(const std::map< std::string, double >& map) { fPCDelayMap = map; };

    protected:
        /**
         * @brief Initializes in-place visibility_type pointer.
         * 
         * @param in Input visibility_type pointer to initialize
         * @return True if initialization succeeds
         * @note This is a virtual function.
         */
        virtual bool InitializeInPlace(visibility_type* in) override;
        /**
         * @brief Initializes out-of-place visibility data from input data.
         * 
         * @param in Const pointer to input visibility_type data.
         * @param out (visibility_type*)
         * @return Boolean indicating success (always true).
         * @note This is a virtual function.
         */
        virtual bool InitializeOutOfPlace(const visibility_type* in, visibility_type* out) override;

        /**
         * @brief Applies manual channel delay corrections in-place for reference or remote station.
         * 
         * @param in Input visibility data containing channel axes.
         * @return True if successful, false otherwise.
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
         * @brief Checks if correction is applicable based on station identity and input visibility.
         * 
         * @param st_idx Index of the station (0 for reference, 1 for remote).
         * @param in Input visibility_type object containing station information.
         * @return Boolean indicating whether correction should be applied.
         */
        bool IsApplicable(std::size_t st_idx, const visibility_type* in);
        /**
         * @brief Checks if the correction polarization matches the polarization product at the given station index [0 = ref, 1 = rem].
         * 
         * @param station_idx Index of the station in polprod string
         * @param polprod Polarization product string
         * @return True if match, false otherwise
         */
        bool PolMatch(std::size_t station_idx, std::string& polprod);
        /**
         * @brief Checks if a given channel label matches an expected one, handling +/- signs for side-band pairs.
         * 
         * @param expected_chan_label Expected channel label to match against
         * @param given_chan_label Given channel label to compare with the expected one
         * @return True if labels match after stripping +/- signs (if needed), false otherwise
         */
        bool LabelMatch(std::string expected_chan_label, std::string given_chan_label);

        //constants
        std::complex< double > fImagUnit;
        double fDegToRad;
        double fNanoSecToSecond;
        double fMHzToHz;
        double fPi;

        //selection
        std::string fStationIdentity;
        std::string fPol;

        //channel label -> pc delay
        std::map< std::string, double > fPCDelayMap;

        //keys for tag retrieval
        std::string fStationKey;
        std::string fRemStationKey;
        std::string fRefStationKey;
        std::string fRemStationMk4IDKey;
        std::string fRefStationMk4IDKey;
        std::string fChannelLabelKey;
        std::string fBandwidthKey;

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

#endif /*! end of include guard: MHO_ManualChannelDelayCorrection */
