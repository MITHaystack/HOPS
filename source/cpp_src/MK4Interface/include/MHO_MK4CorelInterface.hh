#ifndef MHO_MK4CorelInterface_HH__
#define MHO_MK4CorelInterface_HH__

#include <cstdlib>
#include <cstring>
#include <string>
#include <vector>

#include "MHO_ContainerDefinitions.hh"
#include "MHO_MK4VexInterface.hh"

#include "MHO_Message.hh"

//forward declaration of mk4_corel structs
//we do this to keep the mk4 structures from 'leaking' into the new code via includes,
//We want to make sure any interface to the old mk4 IO libraries is kept only
//within the MK4Interface library.
#ifndef HOPS3_USE_CXX
extern "C"
{
#endif

    /**
     * @brief Class mk4_corel
     */
    struct mk4_corel;

#ifndef HOPS3_USE_CXX
}
#endif

namespace hops
{

/*!
 *@file MHO_MK4CorelInterface.hh
 *@class MHO_MK4CorelInterface
 *@author J. Barrett - barrettj@mit.edu
 *@date Thu May 28 19:47:51 2020 -0400
 *@brief MHO_MK4CorelInterface - Needed to import data from mk4_corel file objects (type_1xx) to HOPS4 format.
 * This class implicitly assumes that the frequency/channel configuration
 * is shared among all polarization products (i.e. nlags), we may want to loosen this restriction
 * in the future
 */


class MHO_MK4CorelInterface
{
    public:
        MHO_MK4CorelInterface();
        virtual ~MHO_MK4CorelInterface();

        /**
         * @brief Setter for vex file, we need both the vex (root) file and corel file to extract the data
         * 
         * @param vex Path to the VEX file
         */
        void SetVexFile(const std::string& vex) { fVexFile = vex; }

        /**
         * @brief Setter for corel file
         * 
         * @param corel Input corel file path as string reference
         */
        void SetCorelFile(const std::string& corel) { fCorelFile = corel; }

        /**
         * @brief Getter for raw mk4 corel data struct
         * 
         * @return Pointer to struct mk4_corel
         */
        struct mk4_corel* GetCorelData() { return fCorel; };

        //read the vex and corel files and dump into new format
        /**
         * @brief Extracts Corel file data and stores it in visibility and weight containers.
         */
        void ExtractCorelFile();

        //TODO FIXME:
        //Depending on how the return values are managed by the external caller
        //the return values here are a potential memory leak, so we should
        //consider replacing the raw ptr return value with smart pointers.
        //For now we assume the caller will handle clean-up/deletion, so we do
        //not attempt to delete fExtractedVisibilities/fExtractedWeights in the
        //destructor of this interface class.
        /**
         * @brief Getter for extracted visibilities
         * 
         * @return uch_visibility_store_type* fExtractedVisibilities
         */
        uch_visibility_store_type* GetExtractedVisibilities() { return fExtractedVisibilities; };

        /**
         * @brief Getter for extracted weights
         * 
         * @return uch_weight_store_type* Pointer to the extracted weights.
         */
        uch_weight_store_type* GetExtractedWeights() { return fExtractedWeights; };

    private:
        //corel and vex file members
        /**
         * @brief Reads a Corel file and populates the mk4corel struct.
         */
        void ReadCorelFile();
        /**
         * @brief Reads a VEX file and sets fHaveVex if successful.
         */
        void ReadVexFile();
        bool fHaveCorel;
        bool fHaveVex;
        struct mk4_corel* fCorel;
        mho_json fVex;
        std::string fVexFile;
        std::string fCorelFile;
        std::string fRootCode; //derived from corel file name

        //data dimensions related members
        /**
         * @brief Determines data dimensions for MK4 Corel interface by iterating through indices and channels.
         */
        void DetermineDataDimensions();
        std::size_t fNPPs;
        std::size_t fNAPs;
        std::size_t fNSpectral; //not really number of lags, but rather, spectral points
        std::size_t fNChannels;
        std::size_t fNChannelsPerPP;
        std::set< std::string > fPolProducts;

        //meta-data information
        std::string fBaselineName;      //e.g. Gs:Wf
        std::string fBaselineShortName; //e.g GE
        std::string fRefStation;        //e.g. Gs
        std::string fRemStation;        //e.g. Wf
        std::string fRefStationMk4Id;   //e.g G
        std::string fRemStationMk4Id;   //e.g.E

        //store all channel related data in interval labels for convenience
        std::map< std::string, mho_json > fAllChannelMap;

        //this field stores pointers to channel labels on a per-pol product basis
        //the keys are pol-product labels (e.g. XX, RL, YY, etc)
        //the vectors are sorted by sky-frequency
        std::map< std::string, std::vector< mho_json* > > fPPSortedChannelInfo;

        //helper function to convert raw char arrays to strings
        std::string getstr(const char* char_array, size_t max_size);
        bool channel_info_match(double ref_sky_freq, double rem_sky_freq, double ref_bw, double rem_bw, std::string ref_net_sb,
                                std::string rem_net_sb);
        double calc_freq_bin(double sky_freq, double bw, std::string net_sb, int nlags, int bin_index);

        uch_visibility_store_type* fExtractedVisibilities;
        uch_weight_store_type* fExtractedWeights;
};

} // namespace hops

#endif /*! end of include guard: MHO_MK4CorelInterface */
