#include "HkMK4VexInterface.hh"


namespace hops
{


HkMK4VexInterface::HkMK4VexInterface(){};


HkMK4VexInterface::~HkMK4VexInterface(){};


void
HkMK4VexInterface::OpenVexFile(std::string file_path)
{

    struct vex vex_obj;
    std::string tmp_key(""); //use empty key for now
    int retval = get_vex( const_cast<char*>(file_path.c_str() ),  OVEX | EVEX | IVEX | LVEX , const_cast<char*>(tmp_key.c_str() ), &vex_obj);


    if(retval !=0 )
    {
        std::cout<<"Error: "<< retval <<" when reading file: "<<file_path<<std::endl;
    }
    else
    {
        //do something with the vex object
        std::cout<<"sucessfully read vex root file"<<std::endl;

    }

}


}
