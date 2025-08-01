#ifndef MHO_DiFXVisibilityProcessor_HH__
#define MHO_DiFXVisibilityProcessor_HH__

#include <fstream>
#include <map>
#include <set>
#include <string>
#include <utility>
#include <vector>

#include "MHO_DiFXBaselineProcessor.hh"
#include "MHO_DiFXVisibilityRecord.hh"
#include "MHO_Message.hh"

namespace hops
{

/*!
 *@file  MHO_DiFXVisibilityProcessor.hh
 *@class  MHO_DiFXVisibilityProcessor
 *@author  J. Barrett - barrettj@mit.edu
 *@date Fri Feb 11 12:44:50 2022 -0500
 *@brief  reads a DiFX Swinburne file and extracts the visibilities into a
 * baseline-index mapped set of vectors, and keeps track of the unique pol-pairs
 * seen on each baseline
 */

/**
 * @brief Class MHO_DiFXVisibilityProcessor
 */
class MHO_DiFXVisibilityProcessor
{
    public:
        MHO_DiFXVisibilityProcessor();
        virtual ~MHO_DiFXVisibilityProcessor(){};

        /**
         * @brief Setter for filename
         *
         * @param filename The new filename to set.
         */
        void SetFilename(std::string filename) { fFilename = filename.c_str(); }

        //needed for processing!
        /**
         * @brief Setter for difx .input data
         *
         * @param input Input mho_json object containing DiFX data
         */
        void SetDiFXInputData(const mho_json* input) { fInput = input; }

        /**
         * @brief read the visibilities from Swinburne file and allocate memory to store them as we go
         * memory management of the visibility records is delegated to the caller
         *
         * @param allBaselineVisibilities Reference to std::map containing baseline visibilities
         */
        void ReadDIFXFile(std::map< int, MHO_DiFXBaselineProcessor >& allBaselineVisibilities);

        /**
         * @brief Setter for frequency bands (label, limits)
         *
         * @param fbands Vector of tuples containing band name, lower and upper frequencies
         */
        void SetFrequencyBands(std::vector< std::tuple< std::string, double, double > > fbands) { fFreqBands = fbands; }

        /**
         * @brief Setter for (allowed) freq groups/bands
         *
         * @param fgroups Input vector of frequency group strings
         */
        void SetFreqGroups(std::vector< std::string > fgroups) { fOnlyFreqGroups = fgroups; }

        /**
         * @brief Setter for allow channel bandwidth (only channels with this bandwidth will be kept)
         *
         * @param bw The allowed channel bandwidth value.
         */
        void SetOnlyBandwidth(double bw)
        {
            fOnlyBandwidth = bw;
            fSelectByBandwidth = true;
        }

    private:
        const mho_json* fInput;

        std::string fFilename;
        //maps the pair(baseline,freqindex) to the number of channels (# of spectral points)
        std::map< std::pair< int, int >, int > fNChannelsMap;

        std::vector< std::tuple< std::string, double, double > >
            fFreqBands;                             //frequency band/group labels and ranges (code, flow, fhigh)
        std::vector< std::string > fOnlyFreqGroups; //limit output to matching frequency groups
        bool fSelectByBandwidth;
        double fOnlyBandwidth; //limit output to only channels of this bandwidth
};

} // namespace hops

#endif /*! end of include guard: MHO_DiFXVisibilityProcessor */
