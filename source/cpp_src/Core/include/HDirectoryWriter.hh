#ifndef HDirectoryWriter_HH__
#define HDirectoryWriter_HH__

#include <string>

extern "C"
{
    #include "HBasicDefines.h"
}

namespace hops
{


class HDirectoryWriter
{
    public:
        HDirectoryWriter();
        virtual ~HDirectoryWriter(){};

        void SetExperimentName(std::string exp_name);
        std::string GetExperimentName() const;

        void SetSourceName(std::string source_name);
        std::string GetSourceName() const;

        void SetScanName(std::string scan_name);
        std::string GetScanName() const;

        void SetBaseOutputDirectory(std::string output_dir);
        std::string GetBaseOutputDirectory() const;

        void InitializeOutputDirectory();

        std::string GetCurrentOutputDirectory() const;

    protected:

        std::string fBaseOutputDirectory;
        std::string fCurrentOutputDirectory;

        std::string fExperimentName;
        std::string fSourceName;
        std::string fScanName;
};

}

#endif /* end of include guard: HDirectoryWriter_HH__ */

