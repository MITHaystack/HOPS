#include <string>
#include <iostream>

extern "C"
{
    #include "msg.h"
    #include "vex.h"
    #include "mk4_vex.h"
}

// extern int msglev = 1;
// extern char progname[] = "TestParseVex";

int main(int argc, char** argv)
{
    struct vex root;

    std::string vexfile(argv[1]);

    int ret_val = parse_vexfile( const_cast<char *>(vexfile.c_str()) );

    std::cout<<"parse_vexfile return value = "<<ret_val<<std::endl;

    if (get_vex (const_cast<char *>(vexfile.c_str()), OVEX | EVEX | IVEX | LVEX, "", &root) != 0)
    {
        std::cout<<"Error reading root file: "<<vexfile<<"."<<std::endl;

    }

    std::cout<<"ovex filename = "<<root.ovex->exper_num<<std::endl;
    std::cout<<"acc period = "<<root.evex->ap_length<<std::endl;
    std::cout<<"speed up factor = "<<root.evex->speedup_factor<<std::endl;
    std::cout<<"n stations = "<<root.ovex->nst<<std::endl;
    std::cout<<"station 1 = "<<(root.ovex->st)[0].site_name<<std::endl;

    return 0;
}
