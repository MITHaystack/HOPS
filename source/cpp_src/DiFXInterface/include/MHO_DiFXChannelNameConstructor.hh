#ifndef MHO_DiFXChannelNameConstructor_HH__
#define MHO_DiFXChannelNameConstructor_HH__

#include <map>
#include <sstream>
#include <string>
#include <vector>

#include "MHO_JSONHeaderWrapper.hh"
#include "MHO_Message.hh"

namespace hops
{

/*!
 *@file  MHO_DiFXChannelNameConstructor.hh
 *@class  MHO_DiFXChannelNameConstructor
 *@author  J. Barrett - barrettj@mit.edu
 *@date Tue Jul 12 16:54:42 2022 -0400
 *@brief  Name channels in a vex object, according to the d2m4 convention.
 * Needed in order to convert vex to ovex.
 */

/**
 * @brief Class MHO_DiFXChannelNameConstructor
 */
class MHO_DiFXChannelNameConstructor
{
    public:
        MHO_DiFXChannelNameConstructor();
        virtual ~MHO_DiFXChannelNameConstructor();

        //add a frequency range for a specific band label
        /**
         * @brief Adds a frequency range for a specific band label.
         * 
         * @param band_label Band label as string
         * @param freq_low Lower frequency limit in Hz
         * @param freq_high Upper frequency limit in Hz
         */
        void AddBandLabel(std::string band_label, double freq_low, double freq_high);
        /**
         * @brief Adds channel names to VEX experiment data based on scan and mode information.
         * 
         * @param vex_root Reference to the root JSON object containing VEX experiment data
         */
        void AddChannelNames(mho_json& vex_root);

        //if the (o)vex file has more than one scan, we may want to specify
        //a specific one, otherwise, we will just use the first in the schedule
        /**
         * @brief Setter for scan name
         * 
         * @param scan_id New scan ID to set
         */
        void SetScanName(std::string scan_id) { fScanID = scan_id; }

    private:
        /**
         * @brief Assigns a band label to a given sky frequency.
         * 
         * @param sky_freq Input sky frequency in Hz
         * @return Band label as string
         */
        std::string BandLabelFromSkyFreq(double sky_freq);
        /**
         * @brief Finds channel index by brute force search within tolerance.
         * 
         * @param sky_freq Input sky frequency in Hz.
         * @return Channel index if found within tolerance, else 0.
         */
        std::size_t FindChannelIndex(double sky_freq);

        /**
         * @brief Class band_range
         */
        struct band_range
        {
                double fLow;
                double fHigh;
                std::string fLabel;
        };

        std::vector< band_range > fBandRangeLabels;
        std::string fScanID;

        double fChanTol; //tolerance for labeling disinct frequencies
        std::vector< double > fOrderedSkyFrequencies;
};

} // namespace hops

#endif /*! end of include guard: MHO_DiFXChannelNameConstructor */
