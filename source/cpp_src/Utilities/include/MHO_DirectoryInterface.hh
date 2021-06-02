#ifndef MHO_DirectoryInterface_HH__
#define MHO_DirectoryInterface_HH__

/*
*File: MHO_DirectoryInterface.hh
*Class: MHO_DirectoryInterface
*Author: J. Barrett
*Email: barrettj@mit.edu
*Date:
*Description:
*/

#include <vector>
#include <string>

namespace hops
{

class MHO_DirectoryInterface
{
    public:
        MHO_DirectoryInterface();
        virtual ~MHO_DirectoryInterface();

        std::string GetDirectoryFullPath(const std::string& dirname) const;
        bool DoesDirectoryExist(const std::string& dirname);
        bool CreateDirectory(const std::string& dirname);

        void SetCurrentDirectory(const std::string& dirname);
        std::string GetCurrentDirectory() const;
        std::string GetCurrentParentDirectory() const;

        void ReadCurrentDirectory();

        void GetFileList(std::vector< std::string >& aFileList) const;
        void GetSubDirectoryList(std::vector< std::string >& aSubDirList) const;

        void GetFilesMatchingExtention(std::vector< std::string >& aFileList, const std::string& anExt) const;
        void GetFilesMatchingExtention(std::vector< std::string >& aFileList, const char* anExt) const;

    private:

        std::string get_basename(const std::string& filename) const;
        std::string get_prefix(const std::string& filename) const;

        std::string fCurrentDirectoryFullPath;
        std::string fCurrentParentFullPath;

        bool fDirectoryIsSet;
        bool fHaveReadDirectory;

        std::vector<std::string> fCurrentFileList;
        std::vector<std::string> fCurrentSubDirectoryList;


};


}

#endif /* end of include guard: MHO_DirectoryInterface */
