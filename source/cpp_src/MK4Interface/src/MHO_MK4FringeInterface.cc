#include "MHO_MK4FringeInterface.hh"

#include "MHO_MultiTypeMap.hh"
#include <array>
#include <iostream>
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
}

void MHO_MK4FringeInterface::ExportFringeStructsToJSON(){
    // Call typ200 functions here
    json jsonDump = {{"type_200", MHO_MK4Type200Converter::convertToJSON(fFringe->t200)},
                    {"type_201", MHO_MK4Type201Converter::convertToJSON(fFringe->t201)},
                    {"type_202", MHO_MK4Type202Converter::convertToJSON(fFringe->t202)},
                    {"type_203", MHO_MK4Type203Converter::convertToJSON(fFringe->t203)},
                    {"type_204", MHO_MK4Type204Converter::convertToJSON(fFringe->t204)},
                    {"type_205", MHO_MK4Type205Converter::convertToJSON(fFringe->t205)},
                    {"type_206", MHO_MK4Type206Converter::convertToJSON(fFringe->t206)},
                    {"type_207", MHO_MK4Type207Converter::convertToJSON(fFringe->t207)},
                    {"type_208", MHO_MK4Type208Converter::convertToJSON(fFringe->t208)},
                    {"type_210", MHO_MK4Type210Converter::convertToJSON(fFringe->t210)},
                    {"type_212", MHO_MK4Type212Converter::convertToJSON(fFringe->t212)},
    }

    // Print out fringe file data
    //cout << fFringe;
    std::ofstream o("type-200s-dump.json");
    o << std::setw(4) << jsonDump << std::endl;

    //  
}

void MHO_MK4FringeInterface::ExportFringeFilesToJSON(const std::string& inputFile){
    // call ReadFringeFile
    // call ExportFringeFilesToStructs
    // call ExportFringeStructsToJSON
    //std::cout << inputFile << endl;
    // if (ReadFringeFile(inputFile)) {
        // self.ExportFringeFilesToStructs();
        // self.ExportFringeStructsToJSON();

    //}

}

}
