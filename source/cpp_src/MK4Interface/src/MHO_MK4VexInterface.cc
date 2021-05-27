#include "MHO_MK4VexInterface.hh"

#include "json_wrapper.hh"
#include "MHO_ScanStructWrapper.hh"

namespace hops
{


MHO_MK4VexInterface::MHO_MK4VexInterface()
{
    fVex = (struct vex *) calloc ( 1, sizeof(struct vex) );
};


MHO_MK4VexInterface::~MHO_MK4VexInterface()
{
    free(fVex);
};


void
MHO_MK4VexInterface::OpenVexFile(std::string file_path)
{
    std::string tmp_key(""); //use empty key for now
    int retval = get_vex( const_cast<char*>(file_path.c_str() ),  OVEX | EVEX | IVEX | LVEX , const_cast<char*>(tmp_key.c_str() ), fVex);

    if(retval !=0 )
    {
        fHaveVex = false;
        std::cout<<"Error: "<< retval <<" when reading file: "<<file_path<<std::endl;
    }
    else
    {
        //do something with the vex object
        fHaveVex = true;
        std::cout<<"sucessfully read vex root file"<<std::endl;
    }

}

struct vex*
MHO_MK4VexInterface::GetVex()
{
    if(fHaveVex)
    {
        return fVex;
    }
    else
    {
        return nullptr;
    }
}


bool
MHO_MK4VexInterface::ExportVexFileToJSON(json& json_obj)
{
    if(fHaveVex)
    {
        MHO_ScanStructWrapper scan_wrapper(*(fVex->ovex));
        scan_wrapper.DumpToJSON(json_obj);
        return true;
    }
    else
    {
        return false;
    }
}


}
