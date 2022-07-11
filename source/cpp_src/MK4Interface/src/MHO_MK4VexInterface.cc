#include "MHO_MK4VexInterface.hh"

// #include "MHO_JSONHeaderWrapper.hh"
// #include "MHO_ScanStructWrapper.hh"

#include "MHO_Message.hh"

namespace hops
{


MHO_MK4VexInterface::MHO_MK4VexInterface()
{
    // fVex = (struct vex *) calloc ( 1, sizeof(struct vex) );
    // fOwnVex = true;
    fHaveVex = false;
};


MHO_MK4VexInterface::~MHO_MK4VexInterface()
{
    // if(fOwnVex)
    // {
    //     free(fVex);
    // }
};


// void
// MHO_MK4VexInterface::SetVex(struct vex* root)
// {
//     if(fOwnVex)
//     {
//         free(fVex);
//         fVex = root;
//         fOwnVex = false;
//     }
//     else
//     {
//         //just update the pointer, assume all memory managment is done externally
//         fVex = root;
//     }
// }


void
MHO_MK4VexInterface::OpenVexFile(std::string file_path)
{
    MHO_VexParser vparser;
    vparser.SetVexFile(file_path);
    mho_json fVex = vparser.ParseVex();
    fHaveVex = true;
    // std::string tmp_key(""); //use empty key for now
    // int retval = get_vex( const_cast<char*>(file_path.c_str() ),  OVEX | EVEX | IVEX | LVEX , const_cast<char*>(tmp_key.c_str() ), fVex);
    // 
    // if(retval !=0 )
    // {
    //     fHaveVex = false;
    //     msg_error("mk4interface", "return value: "<< retval  << " encountered when reading file: " << file_path << eom);
    //     // std::cout<<"Error: "<< retval <<" when reading file: "<<file_path<<std::endl;
    // }
    // else
    // {
    //     //do something with the vex object
    //     fHaveVex = true;
    //     msg_info("mk4interface", "successfully read vex file: " << file_path << eom);
    // }

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
        // return nullptr;
    }
}


bool
MHO_MK4VexInterface::ExportVexFileToJSON(json& json_obj)
{
    if(fHaveVex){json_obj = fVex; return true;};
    else{return false;}
    // if(fHaveVex)
    // {
    //     MHO_ScanStructWrapper scan_wrapper(*(fVex->ovex));
    //     scan_wrapper.DumpToJSON(json_obj);
    //     return true;
    // }
    // else
    // {
    //     return false;
    // }
}


}
