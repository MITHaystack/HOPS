#ifndef MHO_MK4StationInterface_HH__
#define MHO_MK4StationInterface_HH__

/*
*File: MHO_MK4StationInterface.hh
*Class: MHO_MK4StationInterface
*Author: J. Barrett
*Email: barrettj@mit.edu
*Date: 2020-05-19T18:54:28.140Z
*Description: This class implicitly assumes that the frequency/channel configuration
* is shared among all polarization products, we may want to loosen this restriction
* in the future
*/

#include <vector>
#include <cstdlib>
#include <cstring>
#include <string>
#include <map>
#include <set>

#include "MHO_ContainerDefinitions.hh"

#include "MHO_Message.hh"

//forward declaration of mk4_corel and vex structs
//we do this to keep the mk4 structures from 'leaking' into the new code via includes,
//We want to make sure any interface to the old mk4 IO libraries is kept only
//within the MK4Interface library (and primarily in the .cc files).
#ifndef HOPS3_USE_CXX
extern "C"
{
#endif
    struct mk4_sdata;
    struct type_309;
#ifndef HOPS3_USE_CXX
}
#endif

namespace hops
{

class MHO_MK4StationInterface
{
    public:

        MHO_MK4StationInterface();
        virtual ~MHO_MK4StationInterface();

        void SetStationFile(const std::string& station){fStationFile = station;}

        mk4_sdata* GetStationData(){return fStation;};

        station_coord_type* ExtractStationFile();

    private:

        std::string
        getstr(const char* char_array, std::size_t max_size)
        {
            return std::string( char_array, std::min( strlen(char_array), max_size) );
        }

        void ReadStationFile();

        //pcal stuff
        void ExtractPCal(int n309, type_309** t309);
        std::string FreqGroupFromMK4ChannelID(std::string id) const;
        std::string PolFromMK4ChannelID(std::string id) const;
        std::string SidebandFromMK4ChannelId(std::string id) const;
        int IndexFromMK4ChannelId(std::string id) const;
        bool ChannelInfoOK(std::string fgroup, std::string sb, std::string pol, int index);

        


        bool fHaveStation;
        struct mk4_sdata* fStation;

        std::string fStationFile;
        std::size_t fNCoeffs;
        std::size_t fNIntervals;
        std::size_t fNCoord;


};

}//end of hops namespace

#endif /* end of include guard: MHO_MK4StationInterface */
