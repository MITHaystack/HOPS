#ifndef MHO_MK4CorelInterfaceReversed_HH__
#define MHO_MK4CorelInterfaceReversed_HH__

#include <cstdlib>
#include <cstring>
#include <string>
#include <vector>
#include <map>

#include "MHO_ContainerDefinitions.hh"
#include "MHO_Message.hh"


//forward declaration of mk4_corel structs
//we do this to keep the mk4 structures from 'leaking' into the new code via includes,
//We want to make sure any interface to the old mk4 IO libraries is kept only
//within the MK4Interface library implementation (.cc) files
#ifndef HOPS3_USE_CXX
extern "C"
{
#endif

    struct mk4_corel;

#ifndef HOPS3_USE_CXX
}
#endif



namespace hops
{

/*!
 *@file MHO_MK4CorelInterfaceReversed.hh
 *@class MHO_MK4CorelInterfaceReversed
 *@author J. Barrett - barrettj@mit.edu
 *@brief MHO_MK4CorelInterfaceReversed - Converts HOPS4 visibility and weight containers back to mk4_corel format.
 */

class MHO_MK4CorelInterfaceReversed
{
    public:
        MHO_MK4CorelInterfaceReversed();
        virtual ~MHO_MK4CorelInterfaceReversed();

        void SetVisibilityData(visibility_store_type* vis_data) { fVisibilityData = vis_data; }
        void SetWeightData(weight_store_type* weight_data) { fWeightData = weight_data; }

        void SetOutputFile(const std::string& output_file) { fOutputFile = output_file; }
        struct mk4_corel* GenerateCorelStructure();

        int WriteCorelFile();

        struct mk4_corel* GetCorelStructure() { return fGeneratedCorel; }

    private:


        void GenerateType000();
        void GenerateType100();
        void GenerateType101Records();
        void GenerateType120Records();
        void InitializeCorelStructure();

        void setstr(const std::string& str, char* char_array, std::size_t max_size);
        //struct date ConvertDateString(const std::string& date_str);

        visibility_store_type* fVisibilityData;
        weight_store_type* fWeightData;
        struct mk4_corel* fGeneratedCorel;
        std::string fOutputFile;

        // Container dimensions
        std::size_t fNPPs;
        std::size_t fNAPs;
        std::size_t fNChannels;
        std::size_t fNSpectral;

        // Channel information extracted from containers
        struct ChannelInfo
        {
            std::string ref_chan_id;
            std::string rem_chan_id;
            std::string pol_product;
            double sky_freq;
            double bandwidth;
            std::string net_sideband;
            int lower_index;
            int upper_index;
        };

        std::vector<ChannelInfo> fChannelInfoList;
        std::map<std::string, std::size_t> fPolProductToIndex;
};

} // namespace hops

#endif /*! end of include guard: MHO_MK4CorelInterfaceReversed */