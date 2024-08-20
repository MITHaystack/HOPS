#include "MHO_MK4FringeInterface.hh"

#include "MHO_MultiTypeMap.hh"
#include <array>
#include <iostream>
#include <fstream>
#include <string>

// TO DO: Decide if I have to add all the type200 header files here or in MK4FringeInterface
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
}

void MHO_MK4FringeInterface::ExportFringeFilesToJSON(const type_200 &t200, const type_201 &t201, const type_202 &t202, const type_203 &t203, const type_204 &t204,
     const type_205 &t205, const type_206 &t206, const type_207 &t207, const type_208 &t208, const type_210 &t210, const type_212 &t212){
    // Call typ200 functions here
    // TODO: change what is passed to the converter funcotions to instances of structs of each of those types.
    json jsonDump = {{"type_200", MHO_MK4Type200Converter::convertToJSON(t200)},
                    {"type_201", MHO_MK4Type201Converter::convertToJSON(t201)},
                    {"type_202", MHO_MK4Type202Converter::convertToJSON(t202)},
                    {"type_203", MHO_MK4Type203Converter::convertToJSON(t203)},
                    {"type_204", MHO_MK4Type204Converter::convertToJSON(t204)},
                    {"type_205", MHO_MK4Type205Converter::convertToJSON(t205)},
                    {"type_206", MHO_MK4Type206Converter::convertToJSON(t206)},
                    {"type_207", MHO_MK4Type207Converter::convertToJSON(t207)},
                    {"type_208", MHO_MK4Type208Converter::convertToJSON(t208)},
                    {"type_210", MHO_MK4Type210Converter::convertToJSON(t210)},
                    {"type_212", MHO_MK4Type212Converter::convertToJSON(t212)},
    }
    // Write fringe file data to file.
    std::ofstream output("type-200s-dump.json");
    output << std::setw(4) << jsonDump << std::endl;
    // std::ofstream file("type-200s-dump.json");
    // file << jsonDump;
    }
}

//void MHO_MK4FringeInterface::ExportFringeFilesToJSON(){
    //jsonDump = ExportFringeStructsToJSON();
    //MHO_MK4FringeInterface::ExportFringeStructsToJSON();

//}
