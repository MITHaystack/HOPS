#include "MHO_MK4VexInterface.hh"

#include "MHO_Message.hh"

namespace hops
{


MHO_MK4VexInterface::MHO_MK4VexInterface()
{
    fHaveVex = false;
};


MHO_MK4VexInterface::~MHO_MK4VexInterface(){};


void
MHO_MK4VexInterface::OpenVexFile(std::string file_path)
{
    MHO_VexParser vparser;
    vparser.SetVexFile(file_path);
    fVex = vparser.ParseVex();
    fHaveVex = true;

}

mho_json
MHO_MK4VexInterface::GetVex()
{
    if(fHaveVex)
    {
        return fVex;
    }
    else
    {
        mho_json empty;
        return empty;
    }
}


bool
MHO_MK4VexInterface::ExportVexFileToJSON(mho_json& json_obj)
{
    if(fHaveVex){json_obj = fVex; return true;}
    else{return false;}
}


}
