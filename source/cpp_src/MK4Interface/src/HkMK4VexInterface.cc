#include "HkMK4VexInterface.hh"


namespace hops
{


HkMK4VexInterface::HkMK4VexInterface()
{
    fVex = (struct vex *) calloc ( 1, sizeof(struct vex) );
};


HkMK4VexInterface::~HkMK4VexInterface()
{
    free(fVex);
};


void
HkMK4VexInterface::OpenVexFile(std::string file_path)
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
HkMK4VexInterface::GetVex()
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


}
