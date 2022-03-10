#include "MHO_DiFXPCalProcessor.hh"

#include <fstream>
#include <cstdlib>

//#include <iostream>

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
    fPCalData.clear();
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
                    //std::cout<<"line = "<<fLine<<std::endl;
                    if(!IsComment())
                    {
                        TokenizeLine();
                        ProcessTokens();
                    }
                }
            }
            file.close();
        }
    }

    //print out the dimensions of the pcal data
    std::cout<<"pcal data size for file: "<<fFilename<<", "<<fPCalData.size()<<std::endl; 

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
        std::vector< pcal_phasor > tone_phasors;
        std::size_t n = 0;
        //break down stuff like:
        //GS 58588.7508738 0.0000116 0 64 7 3480 X  5.16470e-03  3.28259e-02 3475 X  6.48234e-02 -3.06304e-02
        pcal_period pp;
        pp.station = fTokens[n++];
        pp.mjd = std::atof(fTokens[n++].c_str());
        pp.mjd_period = std::atof(fTokens[n++].c_str());
        int place_holder1 = std::atoi(fTokens[n++].c_str()); //TODO FIXE ME -- WHAT IS THIS?
        int place_holder2 = std::atoi(fTokens[n++].c_str()); //TODO FIXE ME -- WHAT IS THIS?
        int place_holder3 = std::atoi(fTokens[n++].c_str()); //TODO FIXE ME -- WHAT IS THIS?
        
        fMJDTimeSet.insert(mjd);

        //now loop through the rest of the p-cal phasor data (4 tokens at a time)
        int itone = 0;
        while(itone >= 0) //last pcal entry in line should start with -1
        {
            itone = std::atoi(fTokens[n].c_str());
            pcal_phasor ph;
            ph.tone_freq = std::atof(fTokens[n++].c_str());
            ph.pol = fTokens[n++];
            fPolSet.insert(ph.pol);
            double real = std::atof(fTokens[n++].c_str());
            double imag = std::atof(fTokens[n++].c_str());
            ph.phasor = std::complex<double>(real,imag);
            tone_phasors.push_back(ph);
        }
        fPCalData.push_back( std::make_pair(pp, tone_phasors) );
    }
}


void 
MHO_DiFXPCalProcessor::Organize()
{
    //we need to run through all of the p-cal data and merge tone/phasor data 
    //from the same time period (can happen w/ multiple datastream-correlation)
    //then stash them in a table container

    

}

}//end of namespace