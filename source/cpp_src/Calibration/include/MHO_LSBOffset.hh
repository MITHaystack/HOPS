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

/**
 * @brief Class MHO_LSBOffset
 */
class MHO_LSBOffset: public MHO_UnaryOperator< visibility_type >
{
    public:
        MHO_LSBOffset();
        virtual ~MHO_LSBOffset();

        /**
         * @brief Setter for station identifier
         *
         * @param station_id mk4 id of type std::string
         * @details station_id is treated as follows:
         * 1-char => mk4 id
         * 2-char => 2char station code
         */
        void SetStationIdentifier(const std::string& id) { fStationIdentities = {id}; }
        void SetStationIdentifiers(const std::vector<std::string>& ids) { fStationIdentities = ids; }

        std::string GetStationIdentifier() const { return fStationIdentities.empty() ? std::string("") : fStationIdentities[0]; }

        /**
         * @brief set lsb (phase) offset for double-sideband channels
         *
         * @param lsb_offset LSB phase offset value
         */
        void SetLSBPhaseOffset(double lsb_offset) { fLSBPhaseOffset = lsb_offset; }

    protected:

        /**
         * @brief Applies LSB phase offset to appropriate channels in-place.
         *
         * @param in Input visibility data.
         * @return True if execution is successful.
         * @note This is a virtual function.
         */
        virtual bool ExecuteInPlace(visibility_type* in) override;

    private:
        /**
         * @brief Checks if correction is applicable based on station index and visibility type.
         *
         * @param st_idx Index of the station
         * @param in Visibility type data
         * @return Boolean indicating whether correction is applicable
         */
        bool IsApplicable(std::size_t st_idx, const visibility_type* in);
        /**
         * @brief Function PolMatch
         *
         * @param station_idx (std::size_t)
         * @param polprod (std::string&)
         * @return Return value (bool)
         */
        bool PolMatch(std::size_t station_idx, std::string& polprod);

        //constants
        std::complex< double > fImagUnit;
        double fDegToRad;

        //selection
        std::vector<std::string> fStationIdentities;

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
