#ifndef MHO_MK4StationInterface_HH__
#define MHO_MK4StationInterface_HH__

#include <cstdlib>
#include <cstring>
#include <map>
#include <set>
#include <string>
#include <vector>

#include "MHO_ContainerDefinitions.hh"
#include "MHO_MK4VexInterface.hh"

#include "MHO_Message.hh"

//forward declaration of mk4_corel and vex structs
//we do this to keep the mk4 structures from 'leaking' into the new code via includes,
//We want to make sure any interface to the old mk4 IO libraries is kept only
//within the MK4Interface library (and primarily in the .cc files).
#ifndef HOPS3_USE_CXX
extern "C"
{
#endif
    /**
     * @brief Class mk4_sdata
     */
    struct mk4_sdata;
    /**
     * @brief Class type_309
     */
    struct type_309;
#ifndef HOPS3_USE_CXX
}
#endif

namespace hops
{

/*!
 *@file MHO_MK4StationInterface.hh
 *@class MHO_MK4StationInterface
 *@author J. Barrett - barrettj@mit.edu
 *@date Fri Nov 13 10:58:21 2020 -0500
 *@brief MHO_MK4StationInterface converts a mk4 station file (type_3xx) to HOPS4 format
 */


class MHO_MK4StationInterface
{
    public:
        MHO_MK4StationInterface();
        virtual ~MHO_MK4StationInterface();

        /**
         * @brief Setter for vex file
         * 
         * @param vex Path to the VEX file
         */
        void SetVexFile(const std::string& vex) { fVexFile = vex; }

        /**
         * @brief Setter for station file
         * 
         * @param station Reference to input station file string
         */
        void SetStationFile(const std::string& station) { fStationFile = station; }

        /**
         * @brief Getter for station data
         * 
         * @return Pointer to mk4_sdata containing station data
         */
        mk4_sdata* GetStationData() { return fStation; };

        /**
         * @brief Extracts station coordinate data from files and allocates memory for it.
         * 
         * @return Pointer to allocated station_coord_type object containing extracted data.
         */
        station_coord_type* ExtractStationFile();

        /**
         * @brief Getter for pcal object
         * 
         * @return Pointer to multitone_pcal_type object
         */
        multitone_pcal_type* GetPCalObject() { return &fAllPCalData; }

    private:
        /**
         * @brief Returns a string from a character array up to max_size.
         * 
         * @param char_array Input character array.
         * @param max_size (std::size_t)
         * @return String representation of input character array up to max_size.
         */
        std::string getstr(const char* char_array, std::size_t max_size)
        {
            return std::string(char_array, std::min(strlen(char_array), max_size));
        }

        /**
         * @brief Reads and processes a station data file.
         */
        void ReadStationFile();
        /**
         * @brief Reads a VEX file and sets fHaveVex if successful.
         */
        void ReadVexFile();

        //pcal stuff
        /**
         * @brief Extracts and constructs PCal data from type_309s.
         * 
         * @param n309 Number of APs (Antennas)
         * @param t309 Array of type_309 pointers
         */
        void ExtractPCal(int n309, type_309** t309);
        /**
         * @brief Fills PCAL array with channel info and calculates delay spline coefficients.
         * 
         * @param pol Polarization string reference
         * @param pol_idx Polarization index
         * @param pc Multitone PCAL type pointer
         * @param n309 Number of T309 structures
         * @param t309 Array of T309 pointers
         */
        void FillPCalArray(const std::string& pol, int pol_idx, multitone_pcal_type* pc, int n309, type_309** t309);

        //builds a visibility channel axis from the ovex info for each pol
        /**
         * @brief Function ConstructPerPolChannelAxis
         * 
         * @return Return value (std::string, channel_axis_type >)
         */
        std::map< std::string, channel_axis_type > ConstructPerPolChannelAxis();

        /**
         * @brief Constructs channel info for MHO MK4StationInterface.
         * 
         * @return std::map<std::string, std::vector<mho_json
         */
        std::map< std::string, std::vector< mho_json > > ConstructChannelInfo();

        //converts a mk4 channel id into its components, returns true if successful
        /**
         * @brief Extracts channel info from MK4 channel ID and populates frequency group, sideband, polarization, and index.
         * 
         * @param ch_name Input MK4 channel ID
         * @param fgroup Output frequency group
         * @param sb Output sideband
         * @param pol Output polarization
         * @param index Output channel index
         * @return True if extraction successful, false otherwise
         */
        bool ExtractChannelInfo(const std::string& ch_name, std::string& fgroup, std::string& sb, std::string& pol, int& index);
        /**
         * @brief Extracts frequency group from MK4 channel ID string.
         * 
         * @param id MK4 channel ID string.
         * @return Frequency group as a string.
         */
        std::string FreqGroupFromMK4ChannelID(std::string id) const;
        /**
         * @brief Extracts polarization from a given MK4 channel ID string.
         * 
         * @param id Input MK4 channel ID string.
         * @return Polarization as a string ('R', 'L', 'X', 'Y', 'H', or 'V').
         */
        std::string PolFromMK4ChannelID(std::string id) const;
        /**
         * @brief Extracts sideband from MK4 channel ID string.
         * 
         * @param id Input MK4 channel ID string
         * @return Extracted sideband as a string ('L', 'U', or 'D') or empty string if invalid
         */
        std::string SidebandFromMK4ChannelId(std::string id) const;
        /**
         * @brief Extracts and converts channel index from MK4 channel ID string.
         * 
         * @param id MK4 channel ID string (e.g., 'X22LY')
         * @return Channel index as integer (-1 if invalid)
         */
        int IndexFromMK4ChannelId(std::string id) const;

        //converts uint32_t counts to complex double
        /**
         * @brief Converts uint32_t counts to complex double for phasor computation.
         * 
         * @param real Real component of input count
         * @param imag Imaginary component of input count
         * @param acc_period Accumulation period in seconds
         * @param sample_period Sample period in seconds
         * @return Complex double representing the computed phasor
         */
        std::complex< double > ComputePhasor(uint32_t real, uint32_t imag, double acc_period, double sample_period);

        //returns a vector of freq group codes found in the 309 data
        std::vector< std::string > GetFreqGroups(int n309, type_309** t309);
        //returns a vector of pols and tone-count for each pol found in 309 data
        std::vector< std::pair< std::string, int > > GetFreqGroupPolInfo(int n309, type_309** t309, const std::string& fg,
                                                                         bool& same_size);

        bool fHaveStation;
        struct mk4_sdata* fStation;
        std::string fStationName;
        std::string fStationCode;
        std::string fStationMK4ID;

        //all pcal data
        multitone_pcal_type fAllPCalData;

        std::string fStationFile;
        std::string fVexFile;
        std::string fRootCode;
        std::size_t fNCoeffs;
        std::size_t fNIntervals;
        std::size_t fNCoord;

        //vex info
        bool fHaveVex;
        mho_json fVex;

        //comparison predicate for sorting channel frequency info
        struct ChannelLess
        {
                bool operator()(const mho_json& a, const mho_json& b) const
                {
                    double a_freq = a["sky_freq"].get< double >();
                    double b_freq = b["sky_freq"].get< double >();
                    double a_bw = a["bandwidth"].get< double >();
                    double b_bw = b["bandwidth"].get< double >();
                    std::string a_sb = a["net_sideband"].get< std::string >();
                    std::string b_sb = b["net_sideband"].get< std::string >();

                    double a_sgn = 0;
                    if(a_sb == "L")
                    {
                        a_sgn = -1.0;
                    }
                    if(a_sb == "U")
                    {
                        a_sgn = 1.0;
                    }

                    double b_sgn = 0;
                    if(b_sb == "L")
                    {
                        b_sgn = -1.0;
                    }
                    if(b_sb == "U")
                    {
                        b_sgn = 1.0;
                    }

                    double a_center_freq = a_freq + a_bw * a_sgn / 2.0;
                    double b_center_freq = b_freq + b_bw * b_sgn / 2.0;

                    return a_center_freq < b_center_freq;
                }
        };

        ChannelLess fChannelPredicate;
};

} // namespace hops

#endif /*! end of include guard: MHO_MK4StationInterface */
