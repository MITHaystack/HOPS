#ifndef MHO_MK4CorelInterfaceReversed_HH__
#define MHO_MK4CorelInterfaceReversed_HH__

#include <cstdlib>
#include <cstring>
#include <string>
#include <vector>

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

        void SetRootFileName(std::string root_filename);
        void SetOutputDirectory(const std::string& output_dir);
        void SetVisibilityData(visibility_store_type* vis_data) { fVisibilityData = vis_data; }
        void SetWeightData(weight_store_type* weight_data) { fWeightData = weight_data; }

        struct mk4_corel* GenerateCorelStructure();
        int WriteCorelFile();

        struct mk4_corel* GetCorelStructure() { return fGeneratedCorel; }

    private:

        std::string ConstructMK4ChannelID(std::string fgroup, int index, std::string sideband, char pol);

        void GenerateType000();
        void GenerateType100();
        void GenerateType101Records();
        void GenerateType120Records();
        void InitializeCorelStructure();

        std::string ConstructType000FileName();

        void setstr(const std::string& str, char* char_array, std::size_t max_size);

        visibility_store_type* fVisibilityData;
        weight_store_type* fWeightData;
        struct mk4_corel* fGeneratedCorel;

        std::string fOutputDir;
        std::string fOutputFile;

        //not read, but used to construct meta-data info in type_000 and type_100
        std::string fRootFilename;
        std::string fRootFileBasename;

        // Container dimensions
        std::size_t fNPPs;
        std::size_t fNAPs;
        std::size_t fNChannels;
        std::size_t fNSpectral;
};

} // namespace hops

#endif /*! end of include guard: MHO_MK4CorelInterfaceReversed */