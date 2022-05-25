#include <string>
#include <iostream>

#include "MHO_JSONHeaderWrapper.hh"

extern "C"
{
    #include "msg.h"
    #include "vex.h"
    #include "mk4_vex.h"
}

#include "MHO_DateStructWrapper.hh"
#include "MHO_SourceStructWrapper.hh"
#include "MHO_SkyCoordStructWrapper.hh"
#include "MHO_StationStructWrapper.hh"
#include "MHO_ScanStructWrapper.hh"

using namespace hops;
// 
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


    json scan_obj;

    scan_obj["exper_num"] = root.ovex->exper_num;
    scan_obj["ap_length"] = root.evex->ap_length;
    scan_obj["speedup_factor"] = root.evex->speedup_factor;
    scan_obj["nst"] = root.ovex->nst;

    std::cout<< scan_obj <<std::endl;

    MHO_DateStructWrapper dwrap(root.ovex->start_time);

    json test;
    dwrap.DumpToJSON(test["start_time"]);

    std::vector<int> stuff;
    stuff.resize(3);
    stuff[0] = 1;
    stuff[1] = 2;
    stuff[2] = 3;

    test["stuff"] = stuff;

    MHO_SourceStructWrapper srcwrap(root.ovex->src);

    srcwrap.DumpToJSON(test["src"]);

    MHO_StationStructWrapper stwrap(root.ovex->st[0]);
    stwrap.DumpToJSON(test["station0"]);

    //std::cout<<test.dump(4)<<std::endl;

    json scan;
    MHO_ScanStructWrapper scan_wrapper(*(root.ovex));
    scan_wrapper.DumpToJSON(scan);

    std::cout<<scan.dump(2)<<std::endl;


    return 0;
}
