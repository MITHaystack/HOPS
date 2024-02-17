#ifndef MHO_DiFXVisibilityProcessor_HH__
#define MHO_DiFXVisibilityProcessor_HH__

/*
*@file: MHO_DiFXVisibilityProcessor.hh
*@class: MHO_DiFXVisibilityProcessor
*@author: J. Barrett
*@email: barrettj@mit.edu
*@date:
*@brief: reads a DiFX Swinburne file and extracts the visibilities into a
* baseline-index mapped set of vectors, and keeps track of the unique pol-pairs
* seen on each baseline
*/

#include <string>
#include <vector>
#include <map>
#include <set>
#include <utility>
#include <fstream>

#include "MHO_Message.hh"
#include "MHO_DiFXVisibilityRecord.hh"
#include "MHO_DiFXBaselineProcessor.hh"


namespace hops
{

class MHO_DiFXVisibilityProcessor
{
    public:

        MHO_DiFXVisibilityProcessor();
        virtual ~MHO_DiFXVisibilityProcessor(){};

        void SetFilename(std::string filename){fFilename = filename.c_str();}

        //needed for processing!
        void SetDiFXInputData(const mho_json* input){fInput = input;}

        //read the visibilities from Swinburne file and allocate memory to store them as we go
        //memory management of the visibility records is delegated to the caller
        void ReadDIFXFile(std::map< int, MHO_DiFXBaselineProcessor >& allBaselineVisibilities);

        void SetFrequencyBands(std::vector< std::tuple<std::string, double, double> > fbands){fFreqBands = fbands;}
        void SetFreqGroups(std::vector< std::string > fgroups){fOnlyFreqGroups = fgroups;}
        void SetOnlyBandwidth(double bw)
        {
            std::cout<<"I AM BEING TOLD TO SELECT BY BANDWIDTH = "<<bw<<std::endl;
            fOnlyBandwidth = bw;
            fSelectByBandwidth = true;
        }

    private:

        const mho_json* fInput;

        std::string fFilename;
        //maps the pair(baseline,freqindex) to the number of channels (# of spectral points)
        std::map< std::pair<int, int>, int> fNChannelsMap;

        std::vector< std::tuple<std::string, double, double> > fFreqBands; //frequency band/group labels and ranges (code, flow, fhigh)
        std::vector< std::string > fOnlyFreqGroups; //limit output to matching frequency groups
        bool fSelectByBandwidth;
        double fOnlyBandwidth; //limit output to only channels of this bandwidth

};


}

#endif /* end of include guard: MHO_DiFXVisibilityProcessor */
