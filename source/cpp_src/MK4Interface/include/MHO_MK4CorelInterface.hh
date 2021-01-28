#ifndef MHO_MK4CorelInterface_HH__
#define MHO_MK4CorelInterface_HH__

/*
*File: MHO_MK4CorelInterface.hh
*Class: MHO_MK4CorelInterface
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

#include "MHO_Visibilities.hh"
#include "MHO_Message.hh"

//forward declaration of mk4_corel and vex structs
//we do this to keep the mk4 structures from 'leaking' into the new code via includes,
//We want to make sure any interface to the old mk4 IO libraries is kept only
//within the MK4Interface library.
extern "C"
{
    struct mk4_corel;
    struct vex;
}

namespace hops
{

class MHO_MK4CorelInterface
{
    public:

        MHO_MK4CorelInterface();
        virtual ~MHO_MK4CorelInterface();

        //need both the vex (root) file and corel file to extract the data
        void SetVexFile(const std::string& vex){fVexFile = vex;}
        void SetCorelFile(const std::string& corel){fCorelFile = corel;}

        //read the vex and corel files and dump into new formatvoid

        baseline_data_type* ExtractCorelFile();

    private:

        //corel and vex file members
        void ReadCorelFile();
        void ReadVexFile();
        bool fHaveCorel;
        bool fHaveVex;
        struct mk4_corel* fCorel;
        struct vex* fVex;
        std::string fVexFile;
        std::string fCorelFile;

        //data dimensions related members
        void DetermineDataDimensions();
        std::size_t fNPPs;
        std::size_t fNAPs;
        std::size_t fNSpectral; //not really number of lags, but rather, spectral points
        std::size_t fNChannels;
        std::size_t fNChannelsPerPP;
        std::set< std::string > fPolProducts;


        //store all channel related data in interval labels for convenience
        std::map< std::string, MHO_IntervalLabel > fAllChannelMap;

        //this field stores pointers to channel labels on a per-pol product basis
        //the keys are pol-product labels (e.g. XX, RL, YY, etc)
        //the vectors are sorted by sky-frequency
        std::map< std::string, std::vector< MHO_IntervalLabel* > > fPPSortedChannelInfo;

        //helper function to convert raw char arrays to strings
        std::string getstr(const char* char_array, size_t max_size);
        bool channel_info_match(double ref_sky_freq, double rem_sky_freq,
                                double ref_bw, double rem_bw,
                                char ref_net_sb, char rem_net_sb);
        double calc_freq_bin(double sky_freq, double bw, char net_sb, int nlags, int bin_index);


};

}//end of hops namespace

#endif /* end of include guard: MHO_MK4CorelInterface */
