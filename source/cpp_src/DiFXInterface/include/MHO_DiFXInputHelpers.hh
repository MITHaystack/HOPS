#ifndef MHO_DiFXInputHelpers_HH__
#define MHO_DiFXInputHelpers_HH__

#include <string>
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

} // namespace MHO_DiFXInputHelpers

} // namespace hops

#endif /* end of include guard: MHO_DiFXInputHelpers_HH__ */
