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

        struct pcal_phasor 
        {
            //std::string pol;
            double tone_freq;
            std::complex<double> phasor;
        };

        struct pcal_period
        {
            std::string station;
            double mjd;
            double mjd_period;
            int ap;
            std::map< std::string, std::vector< pcal_phasor > > polmapped_pcal_phasors; 
        };


        //PCAL data
        std::vector< pcal_period > fPCalData;
        std::vector< pcal_period > fSortedPCalData;
        std::set< std::string> fPolSet;

        struct ToneFreqLess
        {
            bool operator()(const pcal_phasor& a, const pcal_phasor& b) const 
            {
                return (a.tone_freq < b.tone_freq);
            }
        };
        ToneFreqLess fPhasorToneComp;

        struct APIndexLess
        {
            bool operator()(const pcal_period& a, const pcal_period& b) const 
            {
                return (a.ap < b.ap);
            }
        };
        APIndexLess fAPIndexComp;



};

}//end namespace

#endif /* end of include guard: MHO_DiFXPCalProcessor */