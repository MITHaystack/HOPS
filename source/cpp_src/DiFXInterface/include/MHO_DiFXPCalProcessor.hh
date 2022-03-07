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

#include "MHO_Tokenizer.hh"

namespace hops 
{

class MHO_DiFXPCalProcessor
{
    public:
        MHO_DiFXPCalProcessor();
        virtual ~MHO_DiFXPCalProcessor();

        void SetFilename(std::string filename){fFilename = filename;}

        void ReadPCalFile();

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
            double mjd_time;
            double mjd_ap;
            //what other things do we need?
        };

        struct pcal_phasor 
        {
            char pol;
            double tone_freq;
            std::complex<float> phasor;
        };

};

}//end namespace

#endif /* end of include guard: MHO_DiFXPCalProcessor */