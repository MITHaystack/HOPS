#include "MHO_DirectoryInterface.hh"
#include "MHO_Message.hh"

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
    //if -1, failed to create dir -- TBD, do we want to use errno to get more error specifics?
    if(retval == -1){ return false; }
    return true;
}

void
MHO_DirectoryInterface::SetCurrentDirectory(const std::string& dirname)
{
    //set the directory we want to explore
    fCurrentDirectoryFullPath = GetDirectoryFullPath(dirname);
    fCurrentParentFullPath = get_prefix(fCurrentDirectoryFullPath);
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
                            if( st.st_mode & S_IFREG ){allFiles.push_back(fullpath);}
                            //we don't want to collect the current/parent directory, only sub-directories
                            if( fullpath != fCurrentDirectoryFullPath && fullpath != fCurrentParentFullPath)
                            {
                                if( st.st_mode & S_IFDIR ){allDirs.push_back(fullpath);}
                            }
                        }
                    }
                }
            }
            while(epdf != NULL);
        }
        closedir(dpdf);
        fCurrentFileList = allFiles;
        fCurrentSubDirectoryList = allDirs;
        fHaveReadDirectory = true;
    }
    else
    {
        if(fHaveReadDirectory)
        {
            msg_warn("utility", "Already read directory: " << fCurrentDirectoryFullPath << eom);
        }
        if(!fDirectoryIsSet)
        {
            msg_warn("utility", "Attempted to read directory with no path set." << eom);
        }
    }
}

std::string
MHO_DirectoryInterface::get_basename(const std::string& filename) const
{
    std::string base_filename = "";
    std::size_t index = filename.find_last_of("/\\");
    if(index != std::string::npos)
    {
        base_filename = filename.substr(index + 1);
    }
    else
    {
        msg_warn("utility", "No file basename associated with path:" << filename << eom);
    }
    return base_filename;
}

std::string
MHO_DirectoryInterface::get_prefix(const std::string& filename) const
{
    std::string prefix = "";
    std::size_t index = filename.find_last_of("/\\");
    if(index != std::string::npos)
    {
        prefix = filename.substr(0,index);
    }
    else
    {
        msg_warn("utility", "No directory prefix associated with path:" << filename << eom);
    }
    return prefix;
}


void
MHO_DirectoryInterface::GetFilesMatchingExtention(std::vector< std::string >& aFileList, const std::string& anExt) const
{
    //from the current list of files, locate the ones which match the given extension
    aFileList.clear();
    for(auto it = fCurrentFileList.begin(); it != fCurrentFileList.end(); it++)
    {
        std::string basename = get_basename(*it);
        std::size_t index = basename.find_last_of(".");
        if(index != std::string::npos)
        {
            //get the extension
            std::string ext = basename.substr(index);
            if(ext == anExt)
            {
                aFileList.push_back(*it);
            }
        }
    }
}


void
MHO_DirectoryInterface::GetFilesMatchingExtention(std::vector< std::string >& aFileList, const char* anExt) const
{
    //from the current list of files, locate the ones which match the given extension
    aFileList.clear();
    for(auto it = fCurrentFileList.begin(); it != fCurrentFileList.end(); it++)
    {
        std::string basename = get_basename(*it);
        std::size_t index = basename.find_last_of(".");
        if(index != std::string::npos)
        {
            //get the extension
            std::string ext = basename.substr(index+1);
            if(ext == anExt)
            {
                aFileList.push_back(*it);
            }
        }
    }
}

std::string
MHO_DirectoryInterface::GetCurrentDirectory() const
{
    return fCurrentDirectoryFullPath;
}

std::string
MHO_DirectoryInterface::GetCurrentParentDirectory() const
{
    return fCurrentParentFullPath;
}



}
