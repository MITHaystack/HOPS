#ifndef MHO_DirectoryInterface_HH__
#define MHO_DirectoryInterface_HH__

#include <string>
#include <vector>

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

/**
 * @brief Class MHO_DirectoryInterface
 */
class MHO_DirectoryInterface
{
    public:
        MHO_DirectoryInterface();
        virtual ~MHO_DirectoryInterface();

        /**
         * @brief Getter for directory full path
         *
         * @param dirname The directory name to resolve.
         * @return The full absolute path as a string.
         * @note This is a static function.
         */
        static std::string GetDirectoryFullPath(const std::string& dirname);

        /**
         * @brief Getter for directory full path preserve symlinks
         *
         * @param dirname Input directory name
         * @return Full path of the directory as a string
         * @note This is a static function.
         */
        static std::string GetDirectoryFullPathPreserveSymlinks(const std::string& dirname);

        /**
         * @brief Checks if a directory exists by attempting to open it.
         *
         * @param dirname The name of the directory to check.
         * @return True if the directory exists and can be opened, false otherwise.
         * @note This is a static function.
         */
        static bool DoesDirectoryExist(const std::string& dirname);

        /**
         * @brief Checks if a given path is a directory.
         *
         * @param name Path to check.
         * @return True if it's a directory, false otherwise.
         * @note This is a static function.
         */
        static bool IsDirectory(const std::string& name);

        /**
         * @brief Checks if a given path is a regular file.
         *
         * @param name Path to check as std::string&.
         * @return True if it's a regular file, false otherwise.
         * @note This is a static function.
         */
        static bool IsFile(const std::string& name);

        /**
         * @brief Getter for file modifcation time
         *
         * @param name The name of the file to retrieve modification time for.
         * @return A string representing the modification time of the file in timestamp format.
         * @note This is a static function.
         */
        static std::string GetFileModifcationTime(const std::string& name);

        /**
         * @brief Creates a directory with given name and owner permissions.
         *
         * @param dirname The name of the directory to create.
         * @return True if directory creation was successful, false otherwise.
         */
        bool CreateDirectory(const std::string& dirname) const;

        /**
         * @brief Setter for current directory
         *
         * @param dirname The new directory path to set.
         */
        void SetCurrentDirectory(const std::string& dirname);

        /**
         * @brief Getter for current directory
         *
         * @return The full path of the current directory as a std::string.
         */
        std::string GetCurrentDirectory() const;

        /**
         * @brief Getter for current parent directory
         *
         * @return The current parent directory as a std::string.
         */
        std::string GetCurrentParentDirectory() const;

        /**
         * @brief Checks if current directory exists and reads its files and directories.
         *
         * @return bool indicating success/failure.
         */
        bool ReadCurrentDirectory();

        /**
         * @brief Getter for file list
         *
         * @param aFileList Output parameter: vector of strings to store file list
         */
        void GetFileList(std::vector< std::string >& aFileList) const;

        /**
         * @brief Getter for sub directory list
         *
         * @param aSubDirList Output parameter: vector to store subdirectory list
         */
        void GetSubDirectoryList(std::vector< std::string >& aSubDirList) const;

        /**
         * @brief Getter for files matching extention
         *
         * @param aFileList Output vector to store matching file names.
         * @param anExt Extension pattern to match.
         */
        void GetFilesMatchingExtention(std::vector< std::string >& aFileList, const std::string& anExt) const;

        /**
         * @brief Getter for files matching extention
         *
         * @param aFileList Output vector to store matching file names
         * @param anExt Extension to match against
         */
        void GetFilesMatchingExtention(std::vector< std::string >& aFileList, const char* anExt) const;

        /**
         * @brief Getter for files matching prefix
         *
         * @param aFileList Output vector of matching file names
         * @param aPrefix Input string prefix to match against file names
         */
        void GetFilesMatchingPrefix(std::vector< std::string >& aFileList, const std::string& aPrefix) const;

        /**
         * @brief Getter for files matching prefix
         *
         * @param aFileList Output vector of matching file names
         * @param aPrefix Input string prefix to match against file names
         */
        void GetFilesMatchingPrefix(std::vector< std::string >& aFileList, const char* aPrefix) const;

        /**
         * @brief Getter for sub directories matching extention
         *
         * @param aDirList Output vector to store matching directories
         * @param anExt Extension to match for filtering directories
         */
        void GetSubDirectoriesMatchingExtention(std::vector< std::string >& aDirList, const std::string& anExt) const;
        void GetSubDirectoriesMatchingExtention(std::vector< std::string >& aDirList, const char* anExt) const;

        //utility functions for old mk4 format files

        /**
         * @brief extracts legacy mk4 root file from list of files
         */
        void GetRootFile(const std::vector< std::string >& files, std::string& root_file) const;

        /**
         * @brief extracts legacy mk4 corel files from list of files
         */
        void GetCorelFiles(const std::vector< std::string >& files, std::vector< std::string >& corel_files) const;

        /**
         * @brief extracts legacy mk4 station files from list of files
         */
        void GetStationFiles(const std::vector< std::string >& files, std::vector< std::string >& station_files) const;

        /**
         * @brief extracts legacy mk4 fringe files from list of files
         */
        void GetFringeFiles(const std::vector< std::string >& files, std::vector< std::string >& fringe_files,
                            int& max_sequence_num) const;

        /**
         * @brief splits out baseline and root code from basename of a legacy mk4 corel file
         */
        void SplitCorelFileBasename(const std::string& corel_basename, std::string& st_pair, std::string& root_code) const;

        /**
         * @brief splits out station char and root code from basename of a legacy mk4 station file
         */
        void SplitStationFileBasename(const std::string& station_basename, std::string& st, std::string& root_code) const;

        /**
         * @brief gets the file name from a path to a file
         */
        static std::string GetBasename(const std::string& filename);

        /**
         * @brief gets the directory prefix from a full path to a file
         */
        static std::string GetPrefix(const std::string& filename);

        /**
         * @brief removes the extention (anything after last '.') from a file (base) name
         */
        static std::string StripExtensionFromBasename(const std::string& file_basename);

    private:
        //number of chars in a string
        std::size_t count_number_of_matches(const std::string& aString, char elem) const;

        std::string fCurrentDirectoryFullPath;
        std::string fCurrentParentFullPath;

        bool fDirectoryIsSet;
        bool fHaveReadDirectory;

        std::vector< std::string > fCurrentFileList;
        std::vector< std::string > fCurrentSubDirectoryList;

        mutable MHO_Tokenizer fTokenizer;
};

} // namespace hops

#endif /*! end of include guard: MHO_DirectoryInterface */
