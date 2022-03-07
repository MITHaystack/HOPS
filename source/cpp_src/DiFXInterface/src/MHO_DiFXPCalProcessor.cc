#include "MHO_DiFXPCalProcessor.hh"

#include <fstream>
#include <cstdlib>

namespace hops 
{


MHO_DiFXPCalProcessor::MHO_DiFXPCalProcessor()
{
    fTokenizer.SetDelimiter(" ");
    fTokenizer.SetIncludeEmptyTokensFalse();
}


MHO_DiFXPCalProcessor::~MHO_DiFXPCalProcessor(){}

void 
MHO_DiFXPCalProcessor::ReadPCalFile()
{
    if(fFilename != "")
    {
        //open file
        std::ifstream file(fFilename.c_str());

        if(file.is_open())
        {
            //read lines until end 
            while( getline(file,fLine) )
            {
                if(fLine.size() != 0)
                {
                    //parse line and covert tokens into data points 
                    std::cout<<"line = "<<fLine<<std::endl;
                    if(!IsComment(fLine))
                    {
                        
                        TokenizeLine();
                        ProcessTokens();
                    }
                }
            }
            file.close();
        }
    }
}


bool 
MHO_DiFXPCalProcessor::IsComment()
{
    //we are working under the assumption that the header lines have '#' first char
    if(fLine.size() >= 1)
    {
        if(fLine[0] == '#'){return true;}
    }
    return false;
}


void 
MHO_DiFXPCalProcessor::TokenizeLine()
{
    fTokens.clear();
    fTokenizer.SetString(&fLine);
    fTokenizer.GetTokens(&fTokens);
}

void 
MHO_DiFXPCalProcessor::ProcessTokens()
{
    if(fTokens.size() >= 6)
    {
        std::size_t n = 0;
        //break down stuff like:
        //GS 58588.7508738 0.0000116 0 64 7 3480 X  5.16470e-03  3.28259e-02 3475 X  6.48234e-02 -3.06304e-02
        std::string station = fTokens[n++];
        double mjd = std::atof(fTokens[n++].c_str());
        double period = std::atof(fTokens[n++].c_str());
        int place_holder1 = std::atoi(fTokens[n++].c_str()); //TODO FIXE ME -- WHAT IS THIS?
        int place_holder2 = std::atoi(fTokens[n++].c_str()); //TODO FIXE ME -- WHAT IS THIS?
        int place holder3 = std::atoi(fTokens[n++].c_str()); //TODO FIXE ME -- WHAT IS THIS?
        
        //now loop through the rest of the p-cal phasor data (4 tokens at a time)
        int itone = 0;
        while(itone >= 0) //last entry in line should start with -1
        {
            itone = std::atoi(fTokens[n].c_str());
            double tone = std::atof(fTokens[n++].c_str());
            std::string pol = fTokens[n++];
            double real = std::atof(fTokens[n++].c_str());
            double imag = std::atof(fTokens[n++].c_str());
            std::complex<double> phasor(real,imag);
        }


    }
}

}