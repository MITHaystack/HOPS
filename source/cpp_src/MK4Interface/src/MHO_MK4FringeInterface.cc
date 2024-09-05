#include "MHO_MK4FringeInterface.hh"

#include "MHO_MultiTypeMap.hh"
#include <array>
#include <cstdlib>
#include <exception>
#include <iostream>
#include <fstream>
#include <string>

namespace hops
{

template< typename XType, size_t N>
std::array<XType, N> create_and_fill_array(XType values[N])
{
    std::array<XType, N> arr;
    for(size_t i=0; i<N; i++)
    {
        arr[i] = values[i];
    }
    return arr;
}


MHO_MK4FringeInterface::MHO_MK4FringeInterface():
    fHaveFringe(false)
{

}

MHO_MK4FringeInterface::~MHO_MK4FringeInterface()
{
    clear_mk4fringe(&fFringe);
}

void
MHO_MK4FringeInterface::ReadFringeFile(const std::string& filename)
{
    if(fHaveFringe)
    {
        clear_mk4fringe(&fFringe);
    }

    std::string fname = filename;
    int retval = read_mk4fringe( const_cast<char*>(fname.c_str()), &fFringe );
    if(retval == 0)
    {
        fHaveFringe = true;
    }
    else
    {
        fHaveFringe = false;
    }
}

void
MHO_MK4FringeInterface::ExportFringeFilesToStructs()
{
    if(fHaveFringe)
    {
        //want to dump the information in the type_200 through type_230 objects
        //for now just do the POD data types

        MHO_MultiTypeMap< std::string, int, short, float, double, std::array<double, 4>, std::string> _m;

    }
    //replace with the various MHO_MK4Type2XXXConverter from VP
    //convert each type_2xx to json, and insert into a single root json object
}

void MHO_MK4FringeInterface::ExportFringeFilesToJSON(const type_200 &t200, const type_201 &t201, const type_202 &t202, const type_203 &t203, const type_204 &t204,
     const type_205 &t205, const type_206 &t206, const type_207 &t207, const type_208 &t208, const type_210 &t210, const type_212 &t212){
   
    mho_json jsonDump = {{"type_200", convertToJSON(t200)},
                    {"type_201", convertToJSON(t201)},
                    {"type_202", convertToJSON(t202)},
                    {"type_203", convertToJSON(t203)},
                    {"type_204", convertToJSON(t204)},
                    {"type_205", convertToJSON(t205)},
                    {"type_206", convertToJSON(t206)},
                    {"type_207", convertToJSON(t207)},
                    {"type_208", convertToJSON(t208)},
                    {"type_210", convertToJSON(t210)},
                    {"type_212", convertToJSON(t212)},
    };
    
    // Write JSON fringe file data to file.
    std::string homeDir = getenv("HOME"); 
    std::string filePath = homeDir+"/type-200s-dump.json";
    std::cout << "JSON file dumped to:" << filePath << std::endl;
    std::ofstream output(filePath);
    //std::ofstream output("type-200s-dump.json");
    output << std::setw(4) << jsonDump << std::endl;
    }
}
