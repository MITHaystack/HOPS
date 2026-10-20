#ifndef MHO_DiFXInputHelpers_HH__
#define MHO_DiFXInputHelpers_HH__

#include <cctype>
#include <string>
#include <tuple>
#include <utility>
#include <vector>

#include "MHO_JSONHeaderWrapper.hh"

namespace hops
{

namespace MHO_DiFXInputHelpers
{

inline bool IsZoomBand(const mho_json& ds, int band)
{
    return band >= ds.value("nRecBand", 0);
}

inline int GetDatastreamBandFreqId(const mho_json& ds, int band)
{
    int nRecBand = ds.value("nRecBand", 0);
    int nZoomBand = ds.value("nZoomBand", 0);
    if(band < 0 || band >= nRecBand + nZoomBand)
    {
        return -1;
    }

    const char* band_key = "recBandFreqId";
    const char* freq_key = "recFreqId";
    int local_band = band;
    if(band >= nRecBand)
    {
        band_key = "zoomBandFreqId";
        freq_key = "zoomFreqId";
        local_band = band - nRecBand;
    }

    if(!ds.contains(band_key) || !ds.contains(freq_key) || local_band >= (int)ds[band_key].size())
    {
        return -1;
    }
    int local_freq = ds[band_key][local_band].get< int >();
    if(local_freq < 0 || local_freq >= (int)ds[freq_key].size())
    {
        return -1;
    }
    return ds[freq_key][local_freq].get< int >();
}

inline std::string GetDatastreamBandPol(const mho_json& ds, int band)
{
    int nRecBand = ds.value("nRecBand", 0);
    const char* pol_key = "recBandPolName";
    int local_band = band;
    if(band >= nRecBand)
    {
        pol_key = "zoomBandPolName";
        local_band = band - nRecBand;
    }
    if(band < 0 || !ds.contains(pol_key) || local_band >= (int)ds[pol_key].size())
    {
        return "";
    }
    return ds[pol_key][local_band].get< std::string >();
}

inline bool HasTargetFreqs(const mho_json& input)
{
    if(!input.contains("baseline"))
    {
        return false;
    }
    for(const auto& base : input["baseline"])
    {
        if(!base.contains("destFq"))
        {
            return false;
        }
    }
    return true;
}

inline std::vector< int > GetBaselineTargetFreqs(const mho_json& input, int bl)
{
    std::vector< int > target_freqs;
    if(!input.contains("baseline") || bl < 0 || bl >= (int)input["baseline"].size())
    {
        return target_freqs;
    }

    const mho_json& base = input["baseline"][bl];
    if(base.contains("destFq"))
    {
        return base["destFq"].get< std::vector< int > >();
    }

    int nFreq = base.value("nFreq", 0);
    target_freqs.assign(nFreq, -1);
    int dsA = base.value("dsA", -1);
    if(!input.contains("datastream") || dsA < 0 || dsA >= (int)input["datastream"].size() || !base.contains("bandA"))
    {
        return target_freqs;
    }

    const mho_json& ds = input["datastream"][dsA];
    for(int f = 0; f < nFreq && f < (int)base["bandA"].size(); f++)
    {
        if(!base["bandA"][f].empty())
        {
            target_freqs[f] = GetDatastreamBandFreqId(ds, base["bandA"][f][0].get< int >());
        }
    }
    return target_freqs;
}

inline long GetPhaseCalIntervalDivisor(const mho_json& ds)
{
    long divisor = ds.value("phaseCalIntervalDivisor", 1L);
    return (divisor > 0) ? divisor : 1L;
}

inline double GetPhaseCalIntervalMHz(const mho_json& ds)
{
    return ds.value("phaseCalIntervalMHz", 0.0) / (double)GetPhaseCalIntervalDivisor(ds);
}

inline double GetPhaseCalBaseMHz(const mho_json& ds)
{
    return ds.value("phaseCalBaseMHz", 0.0);
}

inline std::string GetDatastreamAntennaName(const mho_json& input, const mho_json& ds)
{
    int antennaId = ds.value("antennaId", -1);
    if(!input.contains("antenna") || antennaId < 0 || antennaId >= (int)input["antenna"].size())
    {
        return "";
    }
    std::string name = input["antenna"][antennaId].value("name", std::string());
    std::size_t len = 0;
    while(len < name.size() && (std::isalnum((unsigned char)name[len]) || name[len] == '_'))
    {
        len++;
    }
    return name.substr(0, len);
}

//band label for a sky frequency (MHz)
//when ranges overlap the narrower band gets precendence
//returns "" on no match
inline std::string DetermineFreqBand(const std::vector< std::tuple< std::string, double, double > >& bands, double freq)
{
    //decide based on sky-freq of channel (we ignore bandwidth/sideband for now)
    //so may not be correct for extra-wide channels...but if that is the case,
    //probably some customized fband settings should be passed by user to specify the freq band labeling

    std::string fband = "";
    double fband_width = 1e30;
    for(const auto& band_tup : bands)
    {
        double low = std::get< 1 >(band_tup);
        double high = std::get< 2 >(band_tup);
        if(high < low)
        {
            std::swap(low, high);
        }
        double width = high - low;
        if(freq < high && low < freq && width < fband_width)
        {
            fband = std::get< 0 >(band_tup);
            fband_width = width;
            //do not break here, so that narrower freq-bands will override larger ones
        }
    }
    return fband;
}

} // namespace MHO_DiFXInputHelpers

} // namespace hops

#endif /* end of include guard: MHO_DiFXInputHelpers_HH__ */
