#ifndef MHO_DirectoryInterface_HH__
#define MHO_DirectoryInterface_HH__


#include <vector>
#include <string>

#include "MHO_Tokenizer.hh"

namespace hops
{

/*!
*@file MHO_DirectoryInterface.hh
*@class MHO_DirectoryInterface
*@author J. Barrett - barrettj@mit.edu
*@date Tue Jun 1 16:14:04 2021 -0400
*@brief implements utility functions to access a directory and locate files
*/


class MHO_DirectoryInterface
{
    public:
        MHO_DirectoryInterface();
        virtual ~MHO_DirectoryInterface();

        static std::string GetDirectoryFullPath(const std::string& dirname);
        static std::string GetDirectoryFullPathPreserveSymlinks(const std::string& dirname);
        static bool DoesDirectoryExist(const std::string& dirname);
        bool CreateDirectory(const std::string& dirname) const;

        void SetCurrentDirectory(const std::string& dirname);
        std::string GetCurrentDirectory() const;
        std::string GetCurrentParentDirectory() const;

        bool ReadCurrentDirectory();

        void GetFileList(std::vector< std::string >& aFileList) const;
        void GetSubDirectoryList(std::vector< std::string >& aSubDirList) const;

        void GetFilesMatchingExtention(std::vector< std::string >& aFileList, const std::string& anExt) const;
        void GetFilesMatchingExtention(std::vector< std::string >& aFileList, const char* anExt) const;

        void GetFilesMatchingPrefix(std::vector< std::string >& aFileList, const std::string& aPrefix) const;
        void GetFilesMatchingPrefix(std::vector< std::string >& aFileList, const char* aPrefix) const;

        void GetSubDirectoriesMatchingExtention(std::vector< std::string >& aDirList, const std::string& anExt) const;
        void GetSubDirectoriesMatchingExtention(std::vector< std::string >& aDirList, const char* anExt) const;

        //utility functions for old mk4 format files
        void GetRootFile(const std::vector<std::string>& files, std::string& root_file) const;
        void GetCorelFiles(const std::vector<std::string>& files, std::vector<std::string>& corel_files) const;
        void GetStationFiles(const std::vector<std::string>& files, std::vector<std::string>& station_files) const;
        void GetFringeFiles(const std::vector<std::string>& files, std::vector<std::string>& fringe_files, int& max_sequence_num) const;
        void SplitCorelFileBasename(const std::string& corel_basename, std::string& st_pair, std::string& root_code) const;
        void SplitStationFileBasename(const std::string& station_basename, std::string& st, std::string& root_code) const;

        static std::string GetBasename(const std::string& filename);
        static std::string GetPrefix(const std::string& filename);
        static std::string StripExtensionFromBasename(const std::string& file_basename);

    private:

        //number of chars in a string
        std::size_t count_number_of_matches(const std::string& aString, char elem) const;

        std::string fCurrentDirectoryFullPath;
        std::string fCurrentParentFullPath;

        bool fDirectoryIsSet;
        bool fHaveReadDirectory;

        std::vector<std::string> fCurrentFileList;
        std::vector<std::string> fCurrentSubDirectoryList;

        mutable MHO_Tokenizer fTokenizer;
};


}

#endif /*! end of include guard: MHO_DirectoryInterface */
