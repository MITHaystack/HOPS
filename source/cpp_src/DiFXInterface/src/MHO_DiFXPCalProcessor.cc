#include "MHO_DiFXPCalProcessor.hh"
#include "MHO_DirectoryInterface.hh"

#include <algorithm>
#include <cmath>
#include <cstdlib>
#include <fstream>
#include <sstream>

//#include <iostream>

namespace hops
{

MHO_DiFXPCalProcessor::MHO_DiFXPCalProcessor()
{
    fTokenizer.SetDelimiter(" ");
    fTokenizer.SetIncludeEmptyTokensFalse();
    fSecondsPerDay = 86400.0;
    fTolerance = 0.01;
    fAPLength = 1.0;
    fScanStartMJD = 0.0;
    fHaveScanStart = false;
    fGridStartMJD = 0.0;
    fValid = false;
}

MHO_DiFXPCalProcessor::~MHO_DiFXPCalProcessor()
{}

void MHO_DiFXPCalProcessor::SetFilename(std::string filename)
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
        msg_error("difx_interface", "filename pattern does not match PCAL type for file: " << filename << eom);
    }

    //reset the tokenizer delim back to the default
    fTokenizer.SetDelimiter(" ");
}

void MHO_DiFXPCalProcessor::ReadPCalFile()
{
    if(fValid)
    {
        fPCalData.clear();
        fPCal.ZeroArray();
        fPolSet.clear();
        fToneFreqs.clear();
        fPresent.clear();
        if(fFilename != "")
        {
            //open file
            std::ifstream file(fFilename.c_str());
            if(file.is_open())
            {
                //read lines until end
                while(getline(file, fLine))
                {
                    if(fLine.size() != 0)
                    {
                        //parse line and covert tokens into data points
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

bool MHO_DiFXPCalProcessor::IsComment()
{
    //we are working under the assumption that the header lines have '#' first char
    if(fLine.size() >= 1)
    {
        if(fLine[0] == '#')
        {
            return true;
        }
    }
    return false;
}

void MHO_DiFXPCalProcessor::TokenizeLine()
{
    fTokens.clear();
    fTokenizer.SetString(&fLine);
    fTokenizer.GetTokens(&fTokens);
}

void MHO_DiFXPCalProcessor::ProcessTokens()
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
        while(n + 4 <= fTokens.size()) //while we have at least 4 tokens left
        {
            itone = std::atoi(fTokens[n].c_str());
            pcal_phasor ph;
            ph.tone_freq = std::atof(fTokens[n++].c_str());
            std::string pol = fTokens[n++];
            double real = std::atof(fTokens[n++].c_str());
            double imag = std::atof(fTokens[n++].c_str());
            ph.phasor = pcal_phasor_type(real, imag);
            if(itone > 0 && pol != "0")
            {
                fPolSet.insert(pol);
                pp.pc_phasors[pol].push_back(ph);
            }
        }

        fPCalData.push_back(pp);
    }
}

void MHO_DiFXPCalProcessor::Organize()
{
    if(!fValid)
    {
        msg_warn("difx_interface", "cannot organize pcal data, invalid state." << eom);
        return;
    }

    std::stringstream ss;
    ss << " has polarizations: ";
    for(auto ppit = fPolSet.begin(); ppit != fPolSet.end(); ppit++)
    {
        ss << *ppit << " ";
    }
    msg_debug("difx_interface", "pcal for station: " << fStationCode << ss.str() << eom);

    //place the pcal records on the scan-relative AP grid (the same grid as the visibilities)
    int first_ap = -1;
    int last_ap = -1;
    if(fPolSet.empty() || !AssignAPIndices(first_ap, last_ap))
    {
        msg_warn("difx_interface", "no usable pcal data for station: " << fStationCode << eom);
        fPCal.Resize(0, 0, 0);
        return;
    }
    BuildToneAxis();

    std::size_t npol = fPolSet.size();
    std::size_t naps = last_ap - first_ap + 1;
    std::size_t ntones = fToneFreqs.size();
    fPCal.Resize(npol, naps, ntones);
    fPCal.ZeroArray();

    std::size_t pol_idx = 0;
    for(auto pol_iter = fPolSet.begin(); pol_iter != fPolSet.end(); pol_iter++)
    {
        std::get< MTPCAL_POL_AXIS >(fPCal).at(pol_idx) = *pol_iter;
        pol_idx++;
    }
    //the time axis is relative to the pcal start time (the centroid of the first AP)
    for(std::size_t ap = 0; ap < naps; ap++)
    {
        std::get< MTPCAL_TIME_AXIS >(fPCal).at(ap) = ap * fAPLength;
    }
    for(std::size_t tone = 0; tone < ntones; tone++)
    {
        std::get< MTPCAL_FREQ_AXIS >(fPCal).at(tone) = fToneFreqs[tone];
    }

    FillPCalTable(first_ap);
    FillMissingPhasors();

    //add axis labels and units
    std::get< MTPCAL_POL_AXIS >(fPCal).Insert(std::string("name"), std::string("polarization"));
    std::get< MTPCAL_TIME_AXIS >(fPCal).Insert(std::string("name"), std::string("time"));
    std::get< MTPCAL_TIME_AXIS >(fPCal).Insert(std::string("units"), std::string("s"));
    std::get< MTPCAL_FREQ_AXIS >(fPCal).Insert(std::string("name"), std::string("frequency"));
    std::get< MTPCAL_FREQ_AXIS >(fPCal).Insert(std::string("units"), std::string("MHz"));

    //add some helpful tags to the fPCal data
    double start_offset_sec = (first_ap + 0.5) * fAPLength;
    double start_time_mjd = fGridStartMJD + start_offset_sec / fSecondsPerDay;
    fPCal.Insert("station_code", fStationCode);
    fPCal.Insert("start_time_mjd", start_time_mjd);

    //convert the mjd time to a sane format
    std::string pcal_start = get_vexdate_from_mjd_sec(fGridStartMJD, start_offset_sec);
    fPCal.Insert("start", pcal_start);

    //insert the pcal accumulation period (the same as the correlation AP)
    fPCal.Insert("pcal_accumulation_period", fAPLength);
}

bool MHO_DiFXPCalProcessor::AssignAPIndices(int& first_ap, int& last_ap)
{
    first_ap = -1;
    last_ap = -1;
    if(fPCalData.empty())
    {
        return false;
    }

    fGridStartMJD = fScanStartMJD;
    if(!fHaveScanStart)
    {
        double first_mjd = fPCalData[0].mjd;
        for(auto it = fPCalData.begin(); it != fPCalData.end(); ++it)
        {
            first_mjd = std::min(first_mjd, it->mjd);
        }
        //pcal records are time-stamped at the AP centroid
        fGridStartMJD = first_mjd - 0.5 * fAPLength / fSecondsPerDay;
        msg_warn("difx_interface", "scan start time not set for pcal data of station: "
                                       << fStationCode << ", assuming the first pcal record is at AP 0" << eom);
    }

    bool period_warned = false;
    std::size_t ndropped = 0;
    for(auto it = fPCalData.begin(); it != fPCalData.end(); ++it)
    {
        double period_sec = fSecondsPerDay * it->mjd_period;
        if(!period_warned && std::fabs(fAPLength - period_sec) / fAPLength > fTolerance)
        {
            msg_warn("difx_interface", "pcal accumulation period ("
                << period_sec << ") of station: " << fStationCode
                << " does not precisely match correlation accumulation period (" << fAPLength
                << "), pcal data will be binned using correlation APs." << eom);
            period_warned = true;
        }

        double dt = (it->mjd - fGridStartMJD) * fSecondsPerDay;
        it->ap = (int)std::floor(dt / fAPLength);
        if(it->ap < 0)
        {
            ndropped++;
            continue;
        }
        if(first_ap < 0 || it->ap < first_ap)
        {
            first_ap = it->ap;
        }
        last_ap = std::max(last_ap, it->ap);
    }

    if(ndropped != 0)
    {
        msg_warn("difx_interface", "dropped " << ndropped << " pcal records preceding the scan start for station: "
            << fStationCode << eom);
    }
    return (first_ap >= 0);
}

void MHO_DiFXPCalProcessor::BuildToneAxis()
{
    fToneFreqs.clear();
    for(auto it = fPCalData.begin(); it != fPCalData.end(); ++it)
    {
        for(auto ppit = it->pc_phasors.begin(); ppit != it->pc_phasors.end(); ++ppit)
        {
            for(auto ph = ppit->second.begin(); ph != ppit->second.end(); ++ph)
            {
                fToneFreqs.push_back(ph->tone_freq);
            }
        }
    }
    std::sort(fToneFreqs.begin(), fToneFreqs.end());
    fToneFreqs.erase(std::unique(fToneFreqs.begin(), fToneFreqs.end()), fToneFreqs.end());
}

std::size_t MHO_DiFXPCalProcessor::FindToneIndex(double tone_freq) const
{
    auto it = std::lower_bound(fToneFreqs.begin(), fToneFreqs.end(), tone_freq);
    if(it != fToneFreqs.end() && *it == tone_freq)
    {
        return it - fToneFreqs.begin();
    }
    return fToneFreqs.size();
}

void MHO_DiFXPCalProcessor::FillPCalTable(int first_ap)
{
    std::size_t npol = fPCal.GetDimension(MTPCAL_POL_AXIS);
    std::size_t naps = fPCal.GetDimension(MTPCAL_TIME_AXIS);
    std::size_t ntones = fToneFreqs.size();

    //records from multiple datastreams covering the same AP are merged here,
    //any (pol, ap, tone) entry reported more than once is averaged
    std::vector< int > counts(npol * naps * ntones, 0);
    for(auto it = fPCalData.begin(); it != fPCalData.end(); ++it)
    {
        if(it->ap < first_ap)
        {
            continue;
        }
        std::size_t ap = it->ap - first_ap;
        std::size_t pol_idx = 0;
        for(auto pol_iter = fPolSet.begin(); pol_iter != fPolSet.end(); pol_iter++, pol_idx++)
        {
            auto ppit = it->pc_phasors.find(*pol_iter);
            if(ppit == it->pc_phasors.end())
            {
                continue;
            }
            for(auto ph = ppit->second.begin(); ph != ppit->second.end(); ++ph)
            {
                std::size_t tone = FindToneIndex(ph->tone_freq);
                fPCal(pol_idx, ap, tone) += ph->phasor;
                counts[ValidIndex(pol_idx, ap, tone)]++;
            }
        }
    }

    fPresent.assign(counts.size(), 0);
    std::size_t nduplicates = 0;
    for(std::size_t pol = 0; pol < npol; pol++)
    {
        for(std::size_t ap = 0; ap < naps; ap++)
        {
            for(std::size_t tone = 0; tone < ntones; tone++)
            {
                int n = counts[ValidIndex(pol, ap, tone)];
                if(n > 1)
                {
                    fPCal(pol, ap, tone) /= (double)n;
                    nduplicates++;
                }
                fPresent[ValidIndex(pol, ap, tone)] = (n > 0);
            }
        }
    }

    if(nduplicates != 0)
    {
        msg_warn("difx_interface", nduplicates << " pcal tone entries of station: " << fStationCode
           << " were reported more than once in the same AP, these have been averaged together."
           << eom);
    }
}

void MHO_DiFXPCalProcessor::FillMissingPhasors()
{
    std::size_t npol = fPCal.GetDimension(MTPCAL_POL_AXIS);
    std::size_t naps = fPCal.GetDimension(MTPCAL_TIME_AXIS);
    std::size_t ntones = fToneFreqs.size();

    std::size_t n_filled = 0;
    std::size_t n_absent = 0;
    std::vector< std::size_t > present_aps;
    for(std::size_t pol = 0; pol < npol; pol++)
    {
        for(std::size_t tone = 0; tone < ntones; tone++)
        {
            present_aps.clear();
            for(std::size_t ap = 0; ap < naps; ap++)
            {
                if(fPresent[ValidIndex(pol, ap, tone)])
                {
                    present_aps.push_back(ap);
                }
            }
            if(present_aps.empty())
            {
                n_absent++; //this tone is never reported for this pol, leave it zero
                continue;
            }

            std::size_t next = 0; //index into present_aps of the first present AP >= ap
            for(std::size_t ap = 0; ap < naps; ap++)
            {
                while(next < present_aps.size() && present_aps[next] < ap)
                {
                    next++;
                }
                if(next < present_aps.size() && present_aps[next] == ap)
                {
                    continue;
                }

                if(next == 0)
                {
                    fPCal(pol, ap, tone) = fPCal(pol, present_aps[0], tone);
                }
                else if(next == present_aps.size())
                {
                    fPCal(pol, ap, tone) = fPCal(pol, present_aps.back(), tone);
                }
                else
                {
                    std::size_t ap0 = present_aps[next - 1];
                    std::size_t ap1 = present_aps[next];
                    double w = (double)(ap - ap0) / (double)(ap1 - ap0);
                    fPCal(pol, ap, tone) = (1.0 - w) * fPCal(pol, ap0, tone) + w * fPCal(pol, ap1, tone);
                }
                n_filled++;
            }
        }
    }

    std::size_t n_missing_aps = 0;
    for(std::size_t ap = 0; ap < naps; ap++)
    {
        bool any = false;
        for(std::size_t pol = 0; pol < npol && !any; pol++)
        {
            for(std::size_t tone = 0; tone < ntones && !any; tone++)
            {
                any = fPresent[ValidIndex(pol, ap, tone)];
            }
        }
        if(!any)
        {
            n_missing_aps++;
        }
    }

    if(n_filled != 0)
    {
        msg_warn("difx_interface", "pcal data of station: " << fStationCode << " has " << n_missing_aps << " of " << naps
            << " APs missing entirely and " << n_filled
            << " missing (pol, AP, tone) entries in total, these have been filled by linear interp in time."
            << eom);
    }
    if(n_absent != 0)
    {
        msg_debug("difx_interface", n_absent << " (pol, tone) combinations of station: " << fStationCode
        << " have no pcal data, treating as zero." << eom);
    }
}

} // namespace hops
