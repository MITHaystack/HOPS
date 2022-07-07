#include "MHO_DiFXPCalProcessor.hh"
#include "MHO_DirectoryInterface.hh"

#include <fstream>
#include <sstream>
#include <cstdlib>
#include <cmath>

//#include <iostream>

namespace hops 
{


MHO_DiFXPCalProcessor::MHO_DiFXPCalProcessor()
{
    fTokenizer.SetDelimiter(" ");
    fTokenizer.SetIncludeEmptyTokensFalse();
    fSecondsPerDay = 86400.0;
    fTolerance = 0.01;
    fValid = false;
}


MHO_DiFXPCalProcessor::~MHO_DiFXPCalProcessor(){}


void 
MHO_DiFXPCalProcessor::SetFilename(std::string filename)
{
    fValid = false;
    //tokenize the file name, and verify it is a 'PCAL' file
    //also extract the station 2-character code 
    fTokenizer.SetDelimiter("_");
    fTokens.clear();

    std::string filename_base = MHO_DirectoryInterface::GetBasename(filename);

    fTokenizer.SetString(&filename_base);
    fTokenizer.GetTokens(&fTokens);

    if(fTokens.size() == 4)
    {
        fType = fTokens[0];
        fMJD_day = fTokens[1];
        fMJD_frac = fTokens[2];
        fStationCode = fTokens[3];
        fFilename = filename;
        fValid = true;
    }
    else 
    {
        msg_error("difx_interface", "filename pattern does not match PCAL type for file: "<< filename << eom);
    }

    //reset the tokenizer delim back to the default
    fTokenizer.SetDelimiter(" ");
}

void 
MHO_DiFXPCalProcessor::ReadPCalFile()
{
    if(fValid)
    {
        fPCalData.clear();
        fSortedPCalData.clear();
        fPCal.ZeroArray();
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
    }
    else 
    {
        msg_warn("difx_interface", "cannot read pcal file in invalid state." << eom);
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
        std::vector< pcal_phasor > tone_phasors;
        std::size_t n = 0;
        //break down stuff like:
        //GS 58588.7508738 0.0000116 0 64 7 3480 X  5.16470e-03  3.28259e-02 3475 X  6.48234e-02 -3.06304e-02
        pcal_period pp;
        pp.station = fTokens[n++];
        pp.mjd = std::atof(fTokens[n++].c_str());
        pp.mjd_period = std::atof(fTokens[n++].c_str());
        int place_holder1 = std::atoi(fTokens[n++].c_str()); //TODO FIXE ME -- WHAT IS THIS PAR?
        int place_holder2 = std::atoi(fTokens[n++].c_str()); //TODO FIXE ME -- WHAT IS THIS PAR?
        int place_holder3 = std::atoi(fTokens[n++].c_str()); //TODO FIXE ME -- WHAT IS THIS PAR?
        pp.pc_phasors.clear();

        //now loop through the rest of the p-cal phasor data (4 tokens at a time)
        int itone = 0;
        while(n < fTokens.size() )
        {
            itone = std::atoi(fTokens[n].c_str());
            pcal_phasor ph;
            ph.tone_freq = std::atof(fTokens[n++].c_str());
            std::string pol = fTokens[n++];
            double real = std::atof(fTokens[n++].c_str());
            double imag = std::atof(fTokens[n++].c_str());
            ph.phasor = pcal_phasor_type(real,imag);
            if(itone > 0 && pol != "0")
            {
                fPolSet.insert(pol);
                pp.pc_phasors[pol].push_back(ph);
            }
        }
        fPCalData.push_back(pp);
    }

}


void 
MHO_DiFXPCalProcessor::Organize()
{
    if(fValid)
    {
        std::stringstream ss;
        ss << " has polarizations: ";
        for(auto ppit = fPolSet.begin(); ppit != fPolSet.end(); ppit++)
        {
            ss << *ppit << " ";
        }

        msg_debug("difx_interface", "pcal for station: "<< fStationCode << ss.str() << eom);

        fSortedPCalData.clear();
        //we need to run through all of the p-cal data and merge tone/phasor data 
        //from the same time period (can happen w/ multiple datastream-correlation)
        //then stash them in a table container

        //first figure out the time of the first AP
        double first_ap = 1e30;
        for(auto it = fPCalData.begin(); it != fPCalData.end(); ++it)
        {
            double ap_time = it->mjd;
            if(ap_time < first_ap){first_ap = ap_time;}
        }

        //then figure out the AP associated with each pcal accumulation
        //note: we are assuming here that the AP length does not change during
        //a scan
        std::set<int> ap_set;
        for(auto it = fPCalData.begin(); it != fPCalData.end(); ++it)
        {
            //check to make sure each pcal AP is the same as the correlation specified AP
            double current_ap_length_sec = fSecondsPerDay*(it->mjd_period);
            if(std::fabs(fAPLength - current_ap_length_sec)/fAPLength > fTolerance )
            {
                msg_warn("difx_interface", "pcal accumulation period ("<<fAPLength<<") does not appear to match correlation accumulation period ("<<current_ap_length_sec<<")." << eom );
            }

            double ap_time = it->mjd;
            double delta = ap_time - first_ap;
            int ap = std::round(delta/(it->mjd_period));
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
                if(fPCalData[idx].ap == ap){ aps_to_merge[ap].push_back(idx);}
            }
        }

        //now merge the pcal data from each ap, and stash in the sorted vector
        for(auto elem = aps_to_merge.begin(); elem != aps_to_merge.end(); elem++)
        {
            int ap = elem->first;
            std::vector<std::size_t> idx_set = elem->second;

            pcal_period pp;
            std::vector< pcal_phasor > ap_pcal;
            for(std::size_t i=0; i<idx_set.size(); i++)
            {
                if(i==0)
                {
                    pp.station = fPCalData[idx_set[i]].station;
                    pp.mjd = fPCalData[idx_set[i]].mjd;
                    pp.mjd_period = fPCalData[idx_set[i]].mjd_period;
                    pp.ap = fPCalData[idx_set[i]].ap;
                    pp.pc_phasors.clear();
                }

                for(auto ppit = fPolSet.begin(); ppit != fPolSet.end(); ppit++)
                {
                    std::string pol = *ppit;
                    pp.pc_phasors[pol].insert( pp.pc_phasors[pol].end(),
                                                           fPCalData[idx_set[i]].pc_phasors[pol].begin(),
                                                           fPCalData[idx_set[i]].pc_phasors[pol].end() );
                }
            }

            for(auto ppit = fPolSet.begin(); ppit != fPolSet.end(); ppit++)
            {
                std::string pol = *ppit;
                std::sort( pp.pc_phasors[pol].begin(), pp.pc_phasors[pol].end(), fPhasorToneComp); 
            }

            fSortedPCalData.push_back(pp);
        }

        //finally sort all by AP 
        std::sort(fSortedPCalData.begin(), fSortedPCalData.end(), fAPIndexComp);

        /* debug print out
        std::cout<<std::setprecision(14)<<std::endl;
        for(auto it = fSortedPCalData.begin(); it != fSortedPCalData.end(); it++)
        {
            std::cout<<"ap, mjd = "<<it->ap<<", "<<it->mjd<<std::endl;
            for(auto ppit = it->pc_phasors.begin(); ppit != it->pc_phasors.end(); ppit++)
            {
                std::cout<<"pol["<<ppit->first<<"], n-tones = "<<ppit->second.size()<<std::endl;
                if(ppit->second.size() != 0)
                {
                    std::cout<<"first tone: "<<ppit->second.begin()->tone_freq<<" last tone: "<<ppit->second.back().tone_freq<<std::endl;
                }
            }
        }
        */
    
        //determine the data dimensions 
        std::size_t npol = fPolSet.size();
        std::size_t naps = fSortedPCalData.size();
        //find the max number of tones
        std::size_t max_ntones = 0;
        std::set<std::size_t> ntone_set;
        for(auto it = fSortedPCalData.begin(); it != fSortedPCalData.end(); it++)
        {
            for(auto ppit = it->pc_phasors.begin(); ppit != it->pc_phasors.end(); ppit++)
            {
                std::size_t current_ntones = ppit->second.size();
                if(current_ntones > max_ntones){max_ntones = current_ntones;}
                ntone_set.insert(current_ntones);
            }
        }

        if(ntone_set.size() != 1)
        {
            std::stringstream sset;
            for(auto it = ntone_set.begin(); it != ntone_set.end(); it++){ sset << *it << ", "; }
            msg_warn("difx_interface", "set of total number of p-cal tones {"<<sset.str()<<"} is inconsistent over polarizations/APs. Incomplete APs will be zero." << eom);
            //std::exit(1); //exit out for now -- TODO figure out how to handle this possible case
        }

        //now we can go ahead and create/resize the organized pcal data-table 
        fPCal.Resize(npol, naps, max_ntones);
        fPCal.ZeroArray();

        //next we set the axes
        std::size_t pol_idx = 0;
        for(auto pol_iter = fPolSet.begin(); pol_iter != fPolSet.end(); pol_iter++)
        {
            std::string pol = *pol_iter;
            std::get<POLPROD_AXIS>(fPCal).at(pol_idx) = pol;
            pol_idx++;
        }

        std::size_t time_idx = 0;
        for(auto it = fSortedPCalData.begin(); it != fSortedPCalData.end(); it++)
        {
            int ap = it->ap;
            std::get<TIME_AXIS>(fPCal).at(time_idx) = ap*fAPLength;
            time_idx++;
        }

        std::size_t tone_idx = 0;
        auto bit = fSortedPCalData[0].pc_phasors[*(fPolSet.begin())].begin();
        auto eit = fSortedPCalData[0].pc_phasors[*(fPolSet.begin())].end();
        for(auto it = bit; it != eit; it++)
        {
            std::get<FREQ_AXIS>(fPCal).at(tone_idx) = it->tone_freq;
            tone_idx++;
        }

        for(pol_idx = 0; pol_idx<npol; pol_idx++)
        {
            std::string pol = std::get<POLPROD_AXIS>(fPCal).at(pol_idx);
            for(time_idx = 0; time_idx<naps; time_idx++)
            {
                auto phasor_vec = &( fSortedPCalData[time_idx].pc_phasors[pol] );

                //We need to check that the phasor vector has the same maximal size,
                //if it is not, we cannot guarantee we have the same set of tones 
                //e.g. if we are missing band S, but have band X, the X-band tones
                //will get shifted into the S-band slots.
                //If we wanted to get fancy we could make a map of tone-value to tone index, 
                //which would let us recover partial Pols/APs. However, given that encountering 
                //partial data is a pretty non-standard situation (or upstream bug), discarding it seems sane.
                if(phasor_vec->size() == max_ntones)
                {
                    for(tone_idx=0; tone_idx < max_ntones; tone_idx++)
                    {
                        fPCal(pol_idx, time_idx, tone_idx) = phasor_vec->at(tone_idx).phasor;
                    }
                }
            }
        }

        //add some helpful tags to the fPCal data;
        fPCal.Insert("station_code", fStationCode);
        fPCal.Insert("start_time_mjd", first_ap);

    }
    else 
    {
        msg_warn("difx_interface", "cannot organize pcal data while in invalid state." << eom);
    }
    //std::cout << fPCal << std::endl;

}

}//end of namespace