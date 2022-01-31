#include "MHO_DiFXInputInterface.hh"

namespace hops
{

MHO_DiFXInputInterface::MHO_DiFXInputInterface():
    fInputDirectory(""),
    fOutputDirectory("")
{};

MHO_DiFXInputInterface::~MHO_DiFXInputInterface(){};

void
MHO_DiFXInputInterface::SetInputDirectory(std::string dir)
{
    fInputDirectory = fDirInterface.GetDirectoryFullPath(dir);
}

void
MHO_DiFXInputInterface::SetOutputDirectory(std::string dir)
{
    fOutputDirectory = fDirInterface.GetDirectoryFullPath(dir);
}

void 
MHO_DiFXInputInterface::Initialize()
{
        //directory interface
        MHO_DirectoryInterface fDirInterface;
        bool in_ok = fDirInterface.DoesDirectoryExist(fInputDirectory);
        bool out_ok = fDirInterface.DoesDirectoryExist(fOutputDirectory);

        msg_info("difx_interface", "input directory: " << fInputDirectory << eom);
        msg_info("difx_interface", "output directory: " << fOutputDirectory << eom);

        //get list of all the files (and directories) in directory
        std::vector< std::string > allFiles;
        std::vector< std::string > allSubDirs;

        fDirInterface.SetCurrentDirectory(fInputDirectory);
        fDirInterface.ReadCurrentDirectory();
        fDirInterface.GetFileList(allFiles);
        fDirInterface.GetSubDirectoryList(allSubDirs);

        //debug
        for(auto it=allFiles.begin(); it != allFiles.end(); it++)
        {
            std::cout<<"file: "<<*it<<std::endl;
        }

        //debug
        for(auto it=allSubDirs.begin(); it != allSubDirs.end(); it++)
        {
            std::cout<<"dir: "<<*it<<std::endl;
        }

        //grab all of the .difx directories 
        std::vector< std::string > difxDirs;
        fDirInterface.GetSubDirectoriesMatchingExtention(difxDirs, "difx");
        for(auto it=difxDirs.begin(); it != difxDirs.end(); it++)
        {
            std::cout<<"difx sub-dir: "<<*it<<std::endl;
        }

        // MHO_DirectoryInterface subfDirInterface;
        // //loop over the difx sub-directories 
        // for(auto it=difxDirs.begin(); it != difxDirs.end(); it++)
        // {
        //     //conver the difx data
        // }

    

}

}//end of namespace