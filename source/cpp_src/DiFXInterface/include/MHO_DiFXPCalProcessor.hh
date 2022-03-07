#ifndef MHO_DiFXPCalProcessor_HH__
#define MHO_DiFXPCalProcessor_HH__

/*
*@file: MHO_DiFXPCalProcessor.hh
*@class: MHO_DiFXPCalProcessor
*@author: J. Barrett
*@email: barrettj@mit.edu
*@date:
*@brief:
*/

#include <string>
#include <vector>
#include <complex>
#include <utility>

#include "MHO_Tokenizer.hh"
#include "MHO_Visibilities.hh"
#include "MHO_ChannelizedVisibilities.hh"

namespace hops 
{

class MHO_DiFXPCalProcessor
{
    public:
        MHO_DiFXPCalProcessor();
        virtual ~MHO_DiFXPCalProcessor();

        void SetFilename(std::string filename){fFilename = filename;}

        void ReadPCalFile();
        void Organize();

    private:

        bool IsComment();
        void TokenizeLine();
        void ProcessTokens();

        std::string fFilename;
        std::string fLine;
        MHO_Tokenizer fTokenizer;
        std::vector< std::string > fTokens;

        struct pcal_period
        {
            std::string station;
            double mjd_start;
            double mjd_period;
            //what other things do we need?
        };

        struct pcal_phasor 
        {
            std::string pol;
            double tone_freq;
            std::complex<double> phasor;
        };

        //PCAL data
        std::vector< std::pair< pcal_period, std::vector< pcal_phasor > > > fPCalData;
        std::set< double > fMJDTimes;

};

}//end namespace

#endif /* end of include guard: MHO_DiFXPCalProcessor */