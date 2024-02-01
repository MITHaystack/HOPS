#include "MHO_DirectoryInterface.hh"
#include "MHO_Message.hh"

//needed for listing/navigating files/directories on *nix
#include <dirent.h>
#include <stdlib.h>
#include <limits.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <fstream>

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
MHO_DirectoryInterface::GetDirectoryFullPath(const std::string& dirname)
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
MHO_DirectoryInterface::DoesDirectoryExist(const std::string& dirname) const
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
MHO_DirectoryInterface::CreateDirectory(const std::string& dirname) const
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
    fCurrentParentFullPath = GetPrefix(fCurrentDirectoryFullPath);
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
MHO_DirectoryInterface::GetBasename(const std::string& filename)
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
MHO_DirectoryInterface::GetPrefix(const std::string& filename)
{
    std::string prefix = "";
    std::size_t index = filename.find_last_of("/\\");
    if(index != std::string::npos)
    {
        prefix = filename.substr(0,index);
    }
    else
    {
        msg_warn("utility", "No directory prefix associated with path: " << filename << eom);
    }
    return prefix;
}


std::string 
MHO_DirectoryInterface::StripExtensionFromBasename(const std::string& file_basename)
{
    //assume we have just the basename (not directory prefix)
    std::string prefix = "";
    std::size_t index = file_basename.find_last_of(".");
    if(index != std::string::npos)
    {
        prefix = file_basename.substr(0,index);
    }
    else
    {
        msg_warn("utility", "No extension to strip from: " << file_basename << eom);
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
        std::string basename = GetBasename(*it);
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
        std::string basename = GetBasename(*it);
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

void 
MHO_DirectoryInterface::GetFilesMatchingPrefix(std::vector< std::string >& aFileList, const std::string& aPrefix) const
{
    //from the current list of files, locate the ones which match the given extension
    aFileList.clear();
    for(auto it = fCurrentFileList.begin(); it != fCurrentFileList.end(); it++)
    {
        std::string basename = GetBasename(*it);
        std::size_t index = basename.find(aPrefix);
        if(index != std::string::npos)
        {
            aFileList.push_back(*it);
        }
    }
}

void 
MHO_DirectoryInterface::GetFilesMatchingPrefix(std::vector< std::string >& aFileList, const char* aPrefix) const
{
    std::string prefix(aPrefix);
    GetFilesMatchingPrefix(aFileList, prefix);
}

void
MHO_DirectoryInterface::GetSubDirectoriesMatchingExtention(std::vector< std::string >& aDirList, const std::string& anExt) const
{
    //from the current list of files, locate the ones which match the given extension
    aDirList.clear();
    for(auto it = fCurrentSubDirectoryList.begin(); it != fCurrentSubDirectoryList.end(); it++)
    {
        std::string basename = GetBasename(*it);
        std::size_t index = basename.find_last_of(".");
        if(index != std::string::npos)
        {
            //get the extension
            std::string ext = basename.substr(index);
            if(ext == anExt)
            {
                aDirList.push_back(*it);
            }
        }
    }
}


void
MHO_DirectoryInterface::GetSubDirectoriesMatchingExtention(std::vector< std::string >& aDirList, const char* anExt) const
{
    //from the current list of files, locate the ones which match the given extension
    aDirList.clear();
    for(auto it = fCurrentSubDirectoryList.begin(); it != fCurrentSubDirectoryList.end(); it++)
    {
        std::string basename = GetBasename(*it);
        std::size_t index = basename.find_last_of(".");
        if(index != std::string::npos)
        {
            //get the extension
            std::string ext = basename.substr(index+1);
            if(ext == anExt)
            {
                aDirList.push_back(*it);
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

void
MHO_DirectoryInterface::GetRootFile(const std::vector<std::string>& files, std::string& root_file) const
{
    //sift through the list of files to find the one which matches the
    //root (ovex) file characteristics (this is relatively primative)
    root_file = "";
    for(auto it = files.begin(); it != files.end(); it++)
    {
        std::string base_filename = it->substr(it->find_last_of("/\\") + 1);
        if(count_number_of_matches(base_filename, '.') == 1) //check that there is one dot in the filename base
        {
            //check to make sure we have 6 character 'root code' extension
            std::string dot(".");
            std::size_t dotpos = base_filename.find(dot);
            if(dotpos != std::string::npos)
            {
                std::string root_code = base_filename.substr(dotpos+1);
                if( root_code.size() == 6 )
                {
                    //open file and check that "VEX" is present in the first few bytes of the file
                    std::fstream test_file(it->c_str(), std::ios::in | std::ios::binary);
                    std::size_t num_bytes = 16;
                    char data[16];
                    for(std::size_t i=0; i<num_bytes; i++)
                    {
                        test_file.read(&(data[i]),1);
                    }
                    data[15] = '\0';
                    std::string test(data);
                    std::string vex("VEX");
                    std::size_t index = test.find(vex);
                    if( index != std::string::npos)
                    {
                        //found an ovex file
                        //TODO FIXME....what happens if there is more then one ovex file?!
                        //may need a warning
                        root_file = *it;
                        return;
                    }
                }
            }

        }
    }
}


void
MHO_DirectoryInterface::GetCorelFiles(const std::vector<std::string>& files, std::vector<std::string>& corel_files) const
{
    corel_files.clear();
    //sift through the list of files to find which ones match the
    //corel file characteristics and put them in the corel_files list
    for(auto it = files.begin(); it != files.end(); it++)
    {
        std::string base_filename = GetBasename(*it); // it->substr(it->find_last_of("/\\") + 1);
        if(count_number_of_matches(base_filename, '.') == 2) //check that there is two dots in the filename base
        {
            std::string st_pair, root_code;
            SplitCorelFileBasename(base_filename, st_pair, root_code);

            //check that the two dots are 'concurrent'
            std::string dots("..");
            std::size_t index = base_filename.find(dots);
            if(st_pair.size() == 2 && root_code.size() == 6 && index != std::string::npos)
            {
                corel_files.push_back(*it);
            }
        }
    }
}


void
MHO_DirectoryInterface::GetStationFiles(const std::vector<std::string>& files, std::vector<std::string>& station_files) const
{
    //sift through the list of files to find the ones which match the
    //station file characteristics
    station_files.clear();
    for(auto it = files.begin(); it != files.end(); it++)
    {
        std::string base_filename = it->substr(it->find_last_of("/\\") + 1);
        if(count_number_of_matches(base_filename, '.') == 2) //check that there is two dots in the filename base
        {
            //check that the two dots are 'concurrent'
            std::string dots("..");
            std::size_t index = base_filename.find(dots);
            if( index != std::string::npos)
            {
                //split the string at the dots into 'station' and 'root code'
                std::string st = base_filename.substr(0,index);
                std::string root_code = base_filename.substr(index+dots.size());
                if(st.size() == 1 && root_code.size() == 6)
                {
                    station_files.push_back(*it);
                }
            }
        }
    }
}

void
MHO_DirectoryInterface::GetFringeFiles(const std::vector<std::string>& files, std::vector<std::string>& fringe_files, int& max_sequence_num) const
{
    //sift through the list of files to find the ones which match the
    //fringe file characteristics
    fringe_files.clear();
    fTokenizer.SetDelimiter(".");
    fTokenizer.SetIncludeEmptyTokensFalse();
    fTokenizer.SetRemoveLeadingTrailingWhitespaceTrue();
    std::vector< std::string > tokens;
    
    max_sequence_num = 0;
    for(auto it = files.begin(); it != files.end(); it++)
    {
        std::string base_filename = it->substr(it->find_last_of("/\\") + 1);
        //check that there is two dots in the filename base
        if(count_number_of_matches(base_filename, '.') == 2) 
        {
            //check that the two dots are separated by a single "frequency group" character
            //format looks like "GE.X.1.0VSI1M"
            tokens.clear();
            fTokenizer.SetString(&base_filename);
            fTokenizer.GetTokens(&tokens);
            if(tokens.size() == 4) 
            {
                //fringe file should get split into 4 tokens
                //first token is the 2-char baseline code 
                //second token is 1-char freq group code 
                //third token is integer "sequence number"
                //4th token is 6 char root code 
                if(tokens[0].size() == 2 && tokens[1].size() == 1 && tokens[3].size() == 6)
                {
                    int seq_no = std::atoi(tokens[2].c_str());
                    if(seq_no > max_sequence_num){max_sequence_num = seq_no;}
                    fringe_files.push_back(*it);
                }
            }
        }
    }
}

void
MHO_DirectoryInterface::SplitCorelFileBasename(const std::string& corel_basename, std::string& st_pair, std::string& root_code) const
{
    st_pair = "";
    root_code = "";
    //check that the two dots are 'concurrent'
    std::string dots("..");
    std::size_t index = corel_basename.find(dots);
    if(index != std::string::npos)
    {
        //split the string at the dots into 'station pair' and 'root code'
        st_pair = corel_basename.substr(0,index);
        root_code = corel_basename.substr(index+dots.size());
    }
}

void
MHO_DirectoryInterface::SplitStationFileBasename(const std::string& station_basename, std::string& st, std::string& root_code) const
{
    st = "";
    root_code = "";
    std::string dots("..");
    std::size_t index = station_basename.find(dots);
    if(index != std::string::npos)
    {
        //split the string at the dots into 'station pair' and 'root code'
        st = station_basename.substr(0,index);
        root_code = station_basename.substr(index+dots.size());
    }
}




std::size_t
MHO_DirectoryInterface::count_number_of_matches(const std::string& aString, char elem) const
{
    std::size_t n_elem = 0;
    for(std::size_t i=0; i<aString.size(); i++)
    {
        if(aString[i] == elem){n_elem++;}
    }
    return n_elem;
}



}
