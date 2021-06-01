#include "MHO_DirectoryInterface.hh"

//needed for listing/navigating files/directories on *nix
#include <dirent.h>
#include <stdlib.h>
#include <limits.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>

namespace hops
{


MHO_DirectoryInterface::MHO_DirectoryInterface():
    fDirectoryIsSet(false),
    fHaveReadDirectory(false)
{
    fCurrentDirectoryFullPath = "";
};

MHO_DirectoryInterface::~MHO_DirectoryInterface(){};

std::string
MHO_DirectoryInterface::GetDirectoryFullPath(const std::string& dirname) const
{
    //get the full path to a directory (in case it is relative)
    std::string tmp_path = dirname;
    char buffer[PATH_MAX];
    for(std::size_t i=0; i<PATH_MAX; i++){buffer[i] = '\0';}
    char* tmp = realpath( tmp_path.c_str(), buffer);
    (void) tmp; //shut up the compiler about unused variable...we don't need it, result is stored in buffer
    std::string fullpath(buffer);
    return fullpath;
}


bool
MHO_DirectoryInterface::DoesDirectoryExist(const std::string& dirname)
{
    std::string fullpath = GetDirectoryFullPath(dirname);
    //check for directory existence by trying to open it, this is not fool-proof
    //but probably close enough until we figure out something better
    bool exists = false;
    DIR* dir = opendir(fullpath.c_str());
    if(dir)
    {
        closedir(dir);
        exists = true;
    }
    return exists;

}

bool
MHO_DirectoryInterface::CreateDirectory(const std::string& dirname)
{
    std::string fullpath = GetDirectoryFullPath(dirname);
    //use mkdir to create the directory with owner permissions
    int retval = mkdir(fullpath.c_str(), S_IRWXU);
    //if -1, failed to create dir -- TBD, do we want to use errno for more specifics?
    if(retval == -1){ return false; }
    return true;
}

void
MHO_DirectoryInterface::SetCurrentDirectory(const std::string& dirname)
{
    //set the directory we want to explore
    fCurrentDirectoryFullPath = GetDirectoryFullPath(dirname);
    fHaveReadDirectory = false;
    fDirectoryIsSet = true;
}

void
MHO_DirectoryInterface::GetFileList(std::vector< std::string >& aFileList) const
{
    aFileList.clear();
    if(fHaveReadDirectory)
    {
        aFileList = fCurrentFileList;
    }
}

void
MHO_DirectoryInterface::GetSubDirectoryList(std::vector< std::string >& aSubDirList) const
{
    aSubDirList.clear();
    if(fHaveReadDirectory)
    {
        aSubDirList = fCurrentSubDirectoryList;
    }
}



void
MHO_DirectoryInterface::ReadCurrentDirectory()
{
    if(fDirectoryIsSet && !fHaveReadDirectory)
    {
        //get list of all the files (and directories) in the directory
        std::vector< std::string > allFiles;
        std::vector< std::string > allDirs;
        DIR *dpdf;
        struct dirent* epdf = NULL;
        dpdf = opendir(fCurrentDirectoryFullPath.c_str());
        if (dpdf != NULL)
        {
            do
            {
                epdf = readdir(dpdf);
                if(epdf != NULL)
                {
                    std::string tmp_path = fCurrentDirectoryFullPath + "/" + std::string(epdf->d_name);
                    char buffer[PATH_MAX];
                    for(std::size_t i=0; i<PATH_MAX; i++){buffer[i] = '\0';}
                    char* tmp = realpath( tmp_path.c_str(), buffer);
                    (void) tmp; //shut up the compiler about unused variable...we don't need it
                    std::string fullpath(buffer);
                    if(fullpath.size() != 0)
                    {
                        struct stat st;
                        if(stat( fullpath.c_str(), &st) == 0 )
                        {
                            if( st.st_mode & S_IFDIR ){allDirs.push_back(fullpath);}
                            if( st.st_mode & S_IFREG ){allFiles.push_back(fullpath);}
                        }
                    }
                }
            }
            while(epdf != NULL);
        }
        closedir(dpdf);
        fCurrentFileList = allFiles;
        fCurrentSubDirectoryList = allDirs;
    }
}


}
