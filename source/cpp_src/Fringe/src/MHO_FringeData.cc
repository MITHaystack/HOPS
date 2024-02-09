#include "MHO_FringeData.hh"

namespace hops 
{

//write data objects to output file...perhaps we may want to move this elsewhere?
int
MHO_FringeData::WriteOutput()
{
    std::string directory = fParameterStore.GetAs<std::string>("/files/directory");
    directory = MHO_DirectoryInterface::GetDirectoryFullPath(directory);

    return 0;
}


}//end namespace