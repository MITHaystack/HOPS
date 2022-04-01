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
        pp.polmapped_pcal_phasors.clear();

        fMJDTimeSet.insert(mjd);

        //now loop through the rest of the p-cal phasor data (4 tokens at a time)
        int itone = 0;
        while(itone >= 0) //last pcal entry in line should start with -1
        {
            itone = std::atoi(fTokens[n].c_str());
            if(itone < 0){break;}
            pcal_phasor ph;
            ph.tone_freq = std::atof(fTokens[n++].c_str());
            std::string pol = fTokens[n++];
            fPolSet.insert(pol);
            double real = std::atof(fTokens[n++].c_str());
            double imag = std::atof(fTokens[n++].c_str());
            ph.phasor = std::complex<double>(real,imag);
            pp.polmapped_pcal_phasors[pol].push_back(ph);
        }
        fPCalData.push_back(pp);
    }
}


void 
MHO_DiFXPCalProcessor::Organize()
{
    fSortedPCalData.clear();
    std::size_t npols == fPolSet.size();
    //we need to run through all of the p-cal data and merge tone/phasor data 
    //from the same time period (can happen w/ multiple datastream-correlation)
    //then stash them in a table container

    //first figure out the time of the first AP
    double first_ap = 1e30;
    for(auto it = fPCalData.begin(); it != fPCalData.end(); ++it)
    {
        double ap_time = it->mjd;
        if(ap_time < first_ap){first_ap = ap_time}
    }

    //then figure out the AP associated with each pcal accumulation
    //note: we are assuming here that the AP length does not change during
    //a scan
    std::set<int> ap_set;
    for(auto it = fPCalData.begin(); it != fPCalData.end(); ++it)
    {
        double ap_time = it->mjd;
        double delta = ap_time - first_ap;
        int ap = delta/(it->mjd_period);
        it->ap = ap;
        ap_set.insert(ap);
    }

    //now we need to merge pcal records that share the same AP
    //brute force search
    std::map< int, std::vector<std::size_t> > aps_to_merge;
    for(auto ap_it = ap_set.begin(); ap_it != ap_set.end(); ++ap_it)
    {
        int ap = *ap_it;
        for(std::size_t idx = 0; idx < fPCalData.size(); ++idx) 
        {
            if(it->ap == ap){ aps_to_merge[ap].push_back(idx);}
        }
    }

    //now merge the pcal data from each ap, and stash in the sorted vector
    for(auto elem = aps_to_merge.being(); elem != aps_to_merge.end(); elem++)
    {
        int ap = elem.first;
        std::vector<std::size_t> idx_set = elem.second;

        pcal_period pp;
        std::vector< pcal_phasor > ap_pcal;
        for(std::size_t i=0; i<idx_set.size(); i++
        {
            if(i==0)
            {
                pp.station = fPCalData[idx_set[i]].station;
                pp.mjd = fPCalData[idx_set[i]].mjd;
                pp.mjd_period = fPCalData[idx_set[i]].mjd_period;
                pp.ap = fPCalData[idx_set[i]].ap;
                pp.polmapped_pcal_phasors.clear();
            }

            for(ppit = fPolSet.begin(); ppit != fPolSet.end(); ppit++)
            {
                std::string pol = *ppit;
                pp.polmapped_pcal_phasors[pol].insert( pp.polmapped_pcal_phasors[pol].end(),
                                                       fPCalData[idx_set[i]].polmapped_pcal_phasors[pol].begin(),
                                                       fPCalData[idx_set[i]].polmapped_pcal_phasors[pol].end() );
            }
        }

        for(ppit = fPolSet.begin(); ppit != fPolSet.end(); ppit++)
        {
            std::string pol = *ppit;
            std::sort( pp.polmapped_pcal_phasors[pol].begin(), pp.polmapped_pcal_phasors[pol].end(), fPhasorToneComp); 
        }

        fSortedPCalData.push_back(pp);
    }

    //finally sort by AP 
    std::sort(fSortedPCalData.begin(), fSortedPCalData.end(), fAPIndexComp);


}

}//end of namespace