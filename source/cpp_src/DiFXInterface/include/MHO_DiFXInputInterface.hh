#ifndef MHO_DiFXInputInterface_HH__
#define MHO_DiFXInputInterface_HH__

/*
*@file: MHO_DiFXInputInterface.hh
*@class: MHO_DiFXInputInterface
*@author: J. Barrett
*@email: barrettj@mit.edu
*@date:
*@brief:
*/

#include <string>
#include <vector>
#include <algorithm>

#include "difxio/difx_input.h"
#include "difxio/parsevis.h"

#include "MHO_Message.hh"
#include "MHO_Tokenizer.hh"

#include "MHO_Visibilities.hh"
#include "MHO_ChannelizedVisibilities.hh"
#include "MHO_VisibilityChannelizer.hh"
#include "MHO_WeightChannelizer.hh"
#include "MHO_StationCoordinates.hh"

#include "MHO_BinaryFileStreamer.hh"
#include "MHO_BinaryFileInterface.hh"
#include "MHO_ClassIdentityMap.hh"

#include "MHO_DirectoryInterface.hh"

#include "MHO_DiFXScanFileSet.hh"

namespace hops
{

class MHO_DiFXInputInterface
{
    public:
        MHO_DiFXInputInterface();
        virtual ~MHO_DiFXInputInterface();

        void SetInputDirectory(std::string dir);
        void SetOutputDirectory(std::string dir);
        void Initialize();

        void ConstructScanFileLists();

    private:

        void ReadDIFX_File(std::string filename);
        void ReadPCAL_File(std::string filename);
        void ReadIM_File(std::string filename);
        void ReadInputFile(std::string filename);

        std::string fInputDirectory;
        std::string fOutputDirectory;

        MHO_DirectoryInterface fDirInterface;

        std::string fVexFile;
        std::string fV2DFile;
        std::vector< MHO_DiFXScanFileSet > fScanFileSetList;

};

}//end of namespace

#endif /* end of include guard: MHO_DiFXInputInterface */