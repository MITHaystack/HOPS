#ifndef MHO_MK4StationInterface_HH__
#define MHO_MK4StationInterface_HH__



#include <vector>
#include <cstdlib>
#include <cstring>
#include <string>
#include <map>
#include <set>

#include "MHO_ContainerDefinitions.hh"
#include "MHO_MK4VexInterface.hh"

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

/*!
*@file MHO_MK4StationInterface.hh
*@class MHO_MK4StationInterface
*@author J. Barrett - barrettj@mit.edu
*@date Fri Nov 13 10:58:21 2020 -0500
*@brief This class implicitly assumes that the frequency/channel configuration
* is shared among all polarization products, we may want to loosen this restriction
* in the future
*/

class MHO_MK4StationInterface
{
    public:

        MHO_MK4StationInterface();
        virtual ~MHO_MK4StationInterface();
        
        void SetVexFile(const std::string& vex){fVexFile = vex;}
        void SetStationFile(const std::string& station){fStationFile = station;}

        mk4_sdata* GetStationData(){return fStation;};

        station_coord_type* ExtractStationFile();

        std::size_t GetNPCalObjects(){return fFreqGroupPCal.size();}
        multitone_pcal_type* GetPCalObject(std::size_t index)
        {
            if(index < fFreqGroupPCal.size() ){ return &(fFreqGroupPCal[index]); }
            else{ return nullptr; }
        };

    private:

        std::string
        getstr(const char* char_array, std::size_t max_size)
        {
            return std::string( char_array, std::min( strlen(char_array), max_size) );
        }

        void ReadStationFile();
        
        void ReadVexFile();

        //pcal stuff
        void ExtractPCal(int n309, type_309** t309);
        void FillPCalArray(const std::string& fgroup, const std::string& pol, int pol_idx, multitone_pcal_type* pc, int n309, type_309** t309);
        //void RepairMK4PCData();
        
        //builds a visibility channel axis from the ovex info
        channel_axis_type ConstructChannelAxis();

        //converts a mk4 channel id into its components, returns true if successful
        bool ExtractChannelInfo(const std::string& ch_name, std::string& fgroup, std::string& sb, std::string& pol, int& index);
        std::string FreqGroupFromMK4ChannelID(std::string id) const;
        std::string PolFromMK4ChannelID(std::string id) const;
        std::string SidebandFromMK4ChannelId(std::string id) const;
        int IndexFromMK4ChannelId(std::string id) const;

        //converts uint32_t counts to complex double
        std::complex< double > ComputePhasor(uint32_t real, uint32_t imag, double acc_period, double sample_period);

        //returns a vector of freq group codes found in the 309 data
        std::vector< std::string > GetFreqGroups(int n309, type_309** t309);
        //returns a vector of pols and tone-count for each pol found in 309 data
        std::vector< std::pair< std::string, int>  > GetFreqGroupPolInfo(int n309, type_309** t309, const std::string& fg, bool& same_size);




        bool fHaveStation;
        struct mk4_sdata* fStation;
        std::string fStationName;
        std::string fStationCode;
        std::string fStationMK4ID;

        //pcal data
        std::vector< multitone_pcal_type > fFreqGroupPCal; //multitone_pcal_type dims = pol x time x freq

        std::string fStationFile;
        std::string fVexFile;
        std::string fRootCode;
        std::size_t fNCoeffs;
        std::size_t fNIntervals;
        std::size_t fNCoord;

        //vex info
        bool fHaveVex;
        mho_json fVex;


};

}//end of hops namespace

#endif /*! end of include guard: MHO_MK4StationInterface */
