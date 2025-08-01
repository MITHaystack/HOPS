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

/**
 * @brief Class MHO_ManualChannelPhaseCorrection
 */
class MHO_ManualChannelPhaseCorrection: public MHO_UnaryOperator< visibility_type >
{
    public:
        MHO_ManualChannelPhaseCorrection();
        virtual ~MHO_ManualChannelPhaseCorrection();

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
         * @brief Setter for polarization
         *
         * @param pol Input polarization string
         */
        void SetPolarization(const std::string& pol)
        {
            fPol = pol;
            make_upper(fPol);
        };

        //channel label -> pc_phases
        /**
         * @brief Setter for channel to pc_phase map
         *
         * @param map Input map of channel labels to phase values
         */
        void SetChannelToPCPhaseMap(const std::map< std::string, double >& map) { fPCMap = map; };

    protected:
        /**
         * @brief Initializes in-place visibility_type pointer.
         *
         * @param in Pointer to visibility_type that will be initialized.
         * @return True if initialization is successful.
         * @note This is a virtual function.
         */
        virtual bool InitializeInPlace(visibility_type* in) override;
        /**
         * @brief Initializes out-of-place data from input visibility_type pointer.
         *
         * @param in Const pointer to input visibility_type data.
         * @param out (visibility_type*)
         * @return Boolean indicating successful initialization.
         * @note This is a virtual function.
         */
        virtual bool InitializeOutOfPlace(const visibility_type* in, visibility_type* out) override;

        /**
         * @brief Applies phase correction to visibility data in-place for reference or remote station.
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
         * @brief Checks if a correction is applicable based on station identity and input visibility data.
         *
         * @param st_idx Index of the station (0 for reference, 1 for remote).
         * @param in Pointer to const visibility_type containing input visibility data.
         * @return Boolean indicating whether a correction should be applied or not.
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
         * @brief Checks if given channel label matches expected label (considering +/- for LSB/USB halves).
         *
         * @param expected_chan_label Expected channel label without +/- for LSB/USB halves
         * @param given_chan_label Given channel label which may contain +/- for LSB/USB halves
         * @return True if given label matches expected after stripping +/- if needed, false otherwise
         */
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
