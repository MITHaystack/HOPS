#include "MHO_MK4Interface.hh"


namespace hops
{


MHO_MK4Interface::MHO_MK4Interface(){};


MHO_MK4Interface::~MHO_MK4Interface(){};

void MHO_MK4Interface::OpenFringeFile(std::string file_path)
{

    struct mk4_fringe fringe;
    int retval = read_mk4fringe( const_cast<char*>( file_path.c_str() ), &fringe);

    if(retval !=0 )
    {
        std::cout<<"Error: "<< retval <<" when reading fringe file."<<std::endl;
    }
    else
    {
        //print the name
        std::string name(fringe.t200->scan_name);
        std::cout<<"name = "<<name<<std::endl;
    }
}




}
