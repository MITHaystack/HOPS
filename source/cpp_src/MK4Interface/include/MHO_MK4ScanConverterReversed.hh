#ifndef MHO_MK4ScanConverterReversed_HH__
#define MHO_MK4ScanConverterReversed_HH__

#include <string>
#include "MHO_ScanDataStore.hh"

//distinguish directory types
#define HOPS4_SCANDIR 0
#define HOPS4_EXPDIR 1
#define HOPS4_UNKNOWNDIR 2


namespace hops
{

/*!
 *@file MHO_MK4ScanConverterReversed.hh
 *@class MHO_MK4ScanConverterReversed
 *@author J. Barrett - barrettj@mit.edu
 *@date Thu Jul 20 13:15:14 2024 -0500
 *@brief Converts HOPS4 format data back to Mark4 directory format (reverse of MHO_MK4ScanConverter)
 */

/**
 * @brief Class MHO_MK4ScanConverterReversed
 */
class MHO_MK4ScanConverterReversed
{
    public:
        MHO_MK4ScanConverterReversed();
        virtual ~MHO_MK4ScanConverterReversed();

        static int DetermineDirectoryType(const std::string& in_dir);

        void ProcessScan(const std::string& input_dir, const std::string& output_dir);

    private:

        void ProcessVex();
        void ProcessCorel();
        void ProcessStation();

        //data  
        MHO_ScanDataStore fStore;
        MHO_DirectoryInterface fDirInterface;
        mho_json fRootJSON;
        std::string fInputDir;
        std::string fOutputDir;
        std::string fOutputVexFile;
    

};

} // namespace hops

#endif /*! end of include guard: MHO_MK4ScanConverterReversed */
