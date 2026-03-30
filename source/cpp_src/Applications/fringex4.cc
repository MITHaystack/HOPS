// fringex4.cc - HOPS4 fringe re-segmentation tool
//
// Reads .frng files produced by fourfit4/fringex4 and re-segments the per-AP
// phasor data into user-specified time intervals with optional delay/rate offsets.
// Outputs A-file records compatible with alist4.
//
// The .frng phasor container stores per-channel (ch=0..nchan-1) fringe-stopped
// complex visibilities, plus a weighted all-channel average at ch=nchan. Axis 0
// contains channel sky frequencies (MHz); axis 1 contains AP start times in
// seconds from scan start.  The fitted residual delay (mbdelay) and rate (drate)
// have already been applied; only additional user-specified offsets need
// correction before coherent summation.

#include <algorithm>
#include <cmath>
#include <complex>
#include <fstream>
#include <iomanip>
#include <iostream>
#include <sstream>
#include <string>
#include <vector>

#include "CLI11.hpp"

#include "MHO_Clock.hh"
#include "MHO_Message.hh"

#include "MHO_BinaryFileInterface.hh"
#include "MHO_ContainerDefinitions.hh"
#include "MHO_ContainerDictionary.hh"
#include "MHO_ObjectTags.hh"
#include "MHO_ParameterStore.hh"

#include "MHO_AFileInfoExtractor.hh"
#include "MHO_DelayModel.hh"

using namespace hops;

// ---------------------------------------------------------------------------
// Per-segment result
// ---------------------------------------------------------------------------
struct SegmentResult
{
        double time_tag_sec; // seconds from scan start to segment center
        double amp;          // amplitude (same units as /fringe/famp)
        double resid_phas;   // residual phase (degrees, -180..+180)
        double total_phas;   // total phase (degrees)
        double snr;          // SNR estimate
        int nap;             // number of APs accumulated
        int duration_sec;    // segment duration in whole seconds
        int start_sec;       // segment start offset from scan start (seconds)
        int offset_sec;      // AP-weighted mean time minus nominal midpoint (seconds)
        // effective total delay/rate used for phase calculation
        double eff_total_mbdelay; // us
        double eff_total_drate;   // us/s
        double resid_mbdelay;     // us: residual + user offset (for MBDLY column)
        double resid_drate;       // ps/s: residual rate + user offset (for DRATE column)
        double resid_delay;       // us: N*AMB + resid_mbdelay to match SBD (for RESIDUALDELAY column)
};

// ---------------------------------------------------------------------------
// A-priori delay model (delay, rate, accel) evaluated at the FRT from splines
// ---------------------------------------------------------------------------
struct AprioriDelayModel
{
        double delay = 0.0; // us
        double rate = 0.0;  // us/s
        double accel = 0.0; // us/s^2
        bool valid = false;
};

// ---------------------------------------------------------------------------
// Parse ref/rem .sta file paths from a .frng file path
// We could probably also construct these from the parameter info in the .frng file
// if that turns out to be more reliable
// Convention:
//   frng: <dir>/GE.Gs-Wf.X.YY.<root>.1.frng
//   ref:  <dir>/G.Gs.<root>.sta
//   rem:  <dir>/E.Wf.<root>.sta
// ---------------------------------------------------------------------------
static bool parse_sta_paths(const std::string& frng_path, std::string& ref_sta_path, std::string& rem_sta_path)
{
    auto slash = frng_path.rfind('/');
    std::string dir = (slash != std::string::npos) ? frng_path.substr(0, slash + 1) : "";
    std::string basename = (slash != std::string::npos) ? frng_path.substr(slash + 1) : frng_path;

    // Split basename on '.' -> ["GE","Gs-Wf","X","YY","47R0AQ","1","frng"]
    std::vector< std::string > parts;
    std::istringstream ss(basename);
    std::string tok;
    while(std::getline(ss, tok, '.'))
    {
        parts.push_back(tok);
    }

    // Need at least 7 fields: baseline, stations, band, pol, root, extent_no, frng
    if(parts.size() < 7 || parts[0].size() < 2)
    {
        msg_error("fringex4", "cannot parse station names from filename: " << basename << eom);
        return false;
    }

    std::string ref_char(1, parts[0][0]);
    std::string rem_char(1, parts[0][1]);

    // Split "Gs-Wf" on '-'
    auto hyphen = parts[1].find('-');
    if(hyphen == std::string::npos)
    {
        msg_error("fringex4", "cannot find '-' in station name field: " << parts[1] << eom);
        return false;
    }
    std::string ref_name = parts[1].substr(0, hyphen);
    std::string rem_name = parts[1].substr(hyphen + 1);
    std::string root = parts[4];

    ref_sta_path = dir + ref_char + "." + ref_name + "." + root + ".sta";
    rem_sta_path = dir + rem_char + "." + rem_name + "." + root + ".sta";
    return true;
}

// ---------------------------------------------------------------------------
// Read station_coord_type from a .sta binary file
// ---------------------------------------------------------------------------
static bool read_sta_file(const std::string& path, station_coord_type& sta_data)
{
    MHO_BinaryFileInterface inter;
    std::vector< MHO_FileKey > keys;
    std::vector< std::size_t > offsets;
    if(!inter.ExtractFileObjectKeysAndOffsets(path, keys, offsets))
    {
        msg_error("fringex4", "failed to read object keys from: " << path << eom);
        return false;
    }

    MHO_ContainerDictionary cdict;
    MHO_UUID sta_uuid = cdict.GetUUIDFor< station_coord_type >();

    for(std::size_t i = 0; i < keys.size(); i++)
    {
        if(keys[i].fTypeId == sta_uuid)
        {
            inter.OpenToReadAtOffset(path, offsets[i]);
            MHO_FileKey key;
            bool ok = inter.Read(sta_data, key);
            inter.Close();
            if(!ok)
            {
                msg_error("fringex4", "failed to read station_coord_type from: " << path << eom);
            }
            return ok;
        }
    }

    msg_error("fringex4", "no station_coord_type object found in: " << path << eom);
    return false;
}

// ---------------------------------------------------------------------------
// Load station spline data and evaluate the a-priori delay model at the FRT.
// Returns false (valid=false) if the .sta files cannot be found or loaded.
// In that case, callers should fall back to the linear (total_mbdelay/total_drate) model.
// ---------------------------------------------------------------------------
static AprioriDelayModel compute_apriori_delay_model(const std::string& frng_path, const MHO_ParameterStore& params)
{
    AprioriDelayModel result;
    result.valid = false;

    std::string ref_sta_path, rem_sta_path;
    if(!parse_sta_paths(frng_path, ref_sta_path, rem_sta_path))
    {
        return result;
    }

    station_coord_type ref_data, rem_data;
    if(!read_sta_file(ref_sta_path, ref_data))
    {
        msg_warn("fringex4", "cannot load ref station file " << ref_sta_path << " - falling back to linear delay model" << eom);
        return result;
    }
    if(!read_sta_file(rem_sta_path, rem_data))
    {
        msg_warn("fringex4", "cannot load rem station file " << rem_sta_path << " - falling back to linear delay model" << eom);
        return result;
    }

    std::string frt_vex = params.GetAs< std::string >("/vex/scan/fourfit_reftime");
    double ref_clockoff = params.GetAs< double >("/ref_station/clock_early_offset"); // usec
    double ref_clockrate = params.GetAs< double >("/ref_station/clock_rate");

    MHO_DelayModel delay_model;
    delay_model.SetFourfitReferenceTimeVexString(frt_vex);
    delay_model.SetReferenceStationData(&ref_data);
    delay_model.SetRemoteStationData(&rem_data);
    delay_model.SetReferenceStationClockOffset(ref_clockoff * 1e-6); // usec -> sec
    delay_model.SetReferenceStationClockRate(ref_clockrate);
    delay_model.ComputeModel();

    // GetDelay/GetRate/GetAcceleration return in sec, sec/sec, sec/sec^2 (spline native units).
    // Convert to us, us/s, us/s^2 to be consistent with /fringe/total_mbdelay and total_drate.
    // TODO formalize units!
    result.delay = delay_model.GetDelay() * 1e6;        // sec -> us
    result.rate = delay_model.GetRate() * 1e6;          // sec/sec -> us/s
    result.accel = delay_model.GetAcceleration() * 1e6; // sec/sec^2 -> us/s^2
    result.valid = true;

    msg_info("fringex4", "a-priori delay model: delay=" << result.delay << " us  rate=" << result.rate
                                                        << " us/s  accel=" << result.accel << " us/s^2" << eom);
    return result;
}

// ---------------------------------------------------------------------------
// Read the phasor data and parameters from a .frng binary file
// Returns false on failure.
// ---------------------------------------------------------------------------
static bool read_frng_file(const std::string& filename, mho_json& base_fsum, phasor_type& phasors, MHO_ParameterStore& params)
{
    // --- Use AFileInfoExtractor to get the standard summary fields ---
    MHO_AFileInfoExtractor extractor;
    if(!extractor.SummarizeFringeFile(filename, base_fsum))
    {
        msg_error("fringex4", "failed to summarize fringe file: " << filename << eom);
        return false;
    }

    // --- Scan the binary file for named objects ---
    MHO_BinaryFileInterface inter;
    std::vector< MHO_FileKey > keys;
    std::vector< std::size_t > offsets;
    if(!inter.ExtractFileObjectKeysAndOffsets(filename, keys, offsets))
    {
        msg_error("fringex4", "failed to read object keys from: " << filename << eom);
        return false;
    }

    // --- Find and read MHO_ObjectTags (by type UUID) to populate params ---
    MHO_ContainerDictionary cdict;
    MHO_UUID tag_uuid = cdict.GetUUIDFor< MHO_ObjectTags >();

    bool found_tags = false;
    std::size_t tags_offset = 0;
    for(std::size_t i = 0; i < keys.size(); i++)
    {
        if(keys[i].fTypeId == tag_uuid)
        {
            tags_offset = offsets[i];
            found_tags = true;
            break;
        }
    }

    if(!found_tags)
    {
        msg_error("fringex4", "no MHO_ObjectTags found in: " << filename << eom);
        return false;
    }

    inter.OpenToReadAtOffset(filename, tags_offset);
    MHO_ObjectTags tags_obj;
    MHO_FileKey key;
    if(!inter.Read(tags_obj, key))
    {
        msg_error("fringex4", "failed to read MHO_ObjectTags from: " << filename << eom);
        inter.Close();
        return false;
    }
    inter.Close();

    mho_json param_data;
    if(!tags_obj.GetTagValue("parameters", param_data))
    {
        msg_error("fringex4", "no 'parameters' tag in MHO_ObjectTags: " << filename << eom);
        return false;
    }
    params.FillData(param_data);

    // --- Find and read phasor_type object by short name "phasors" ---
    bool found_phasors = false;
    std::size_t phasors_offset = 0;
    for(std::size_t i = 0; i < keys.size(); i++)
    {
        std::string kname(keys[i].fName);
        if(kname == "phasors")
        {
            phasors_offset = offsets[i];
            found_phasors = true;
            break;
        }
    }

    if(!found_phasors)
    {
        msg_error("fringex4", "no 'phasors' object found in: " << filename << eom);
        return false;
    }

    inter.OpenToReadAtOffset(filename, phasors_offset);
    MHO_FileKey pkey;
    if(!inter.Read(phasors, pkey))
    {
        msg_error("fringex4", "failed to read phasors from: " << filename << eom);
        inter.Close();
        return false;
    }
    inter.Close();

    return true;
}

// ---------------------------------------------------------------------------
// Coherently accumulate phasors into time segments and compute per-segment stats
//
// Phase convention:
//   The stored phasors have the fitted (drate + total_drate) and
//   (mbdelay + total_mbdelay) already removed.  To apply an additional
//   user-specified delta correction, multiply each AP's phasor by:
//
//     correction(ch, ap) = exp(-2*pi*i * [freq_ch * eff_dr * tdelta
//                                     + eff_mbd * (freq_ch - ref_freq)])
//
//   where freq_ch (MHz), eff_dr (us/s), tdelta (s), eff_mbd (us).
//   Units: MHz*us/s*s = MHz*us = 10^6*10^{-6} = dimensionless (cycles).
//
//   When eff_dr==0 and eff_mbd==0 the correction is unity and the per-channel
//   phasors are summed directly (equal weights, 1/nchan normalisation).
// ---------------------------------------------------------------------------
static std::vector< SegmentResult > compute_segments(const phasor_type& phasors, const MHO_ParameterStore& params,
                                                     double nsecs,                // segment length (s); <=0 -> full scan
                                                     double user_drate,           // us/s: offset or absolute (cmode)
                                                     double user_mbdelay,         // us:   offset or absolute (cmode)
                                                     bool cmode,                  // true -> user values are absolute
                                                     bool overlap,                // true -> half-step interleaved windows
                                                     const AprioriDelayModel& dm) // a-priori delay model from splines
{
    // --- Parameters from file ---
    double ap_period = params.GetAs< double >("/config/ap_period");       // s
    double frt_offset = params.GetAs< double >("/config/frt_offset");     // s from scan start
    double ref_freq = params.GetAs< double >("/control/config/ref_freq"); // MHz
    double fit_mbdelay = params.GetAs< double >("/fringe/mbdelay");       // us (residual only)
    // /fringe/drate is stored in us/s; multiply by 1e6 only at output time for the ps/s column
    double fit_drate = params.GetAs< double >("/fringe/drate");             // us/s (residual only)
    double total_mbdelay = params.GetAs< double >("/fringe/total_mbdelay"); // us (a-priori + residual)
    double total_drate = params.GetAs< double >("/fringe/total_drate");     // us/s (a-priori + residual)
    double full_snr = params.GetAs< double >("/fringe/snr");
    double fourfit_amp = params.GetAs< double >("/fringe/famp");
    // /fringe/resid_delay = N*AMB + MBD (integer-ambiguity-adjusted MBD to match SBD),
    // computed by fourfit4 and stored in the frng file.
    double file_resid_delay = params.GetAs< double >("/fringe/resid_delay"); // us

    // total_mbdelay/total_drate already include fit_mbdelay/fit_drate (no double-counting)
    double applied_mbdelay = total_mbdelay; // us
    double applied_drate = total_drate;     // us/s

    // Effective delta correction to apply to the already-stopped phasors
    double eff_dr, eff_mbd;
    if(cmode)
    {
        // user values are absolute total; delta = user - applied
        eff_dr = user_drate - applied_drate;
        eff_mbd = user_mbdelay - applied_mbdelay;
    }
    else
    {
        // user values are additional offsets
        eff_dr = user_drate;
        eff_mbd = user_mbdelay;
    }

    // Effective total delay and rate to report in TOTMBDELAY/TOTDRATE columns
    double eff_total_drate = applied_drate + eff_dr;
    double eff_total_mbdelay = applied_mbdelay + eff_mbd;

    // Residual + user offset for MBDLY/DRATE columns (mirrors fringex: delayoff = resid + user)
    // resid_mbdelay in us; resid_drate converted us/s -> ps/s for the DRATE output column
    double resid_mbdelay = fit_mbdelay + eff_mbd;
    double resid_drate = (fit_drate + eff_dr) * 1e6; // us/s -> ps/s

    // --- Phasor dimensions ---
    std::size_t nchan_plus1 = phasors.GetDimension(0); // nchan real channels + 1 "All"
    std::size_t nap = phasors.GetDimension(1);
    std::size_t nchan = nchan_plus1 > 0 ? nchan_plus1 - 1 : 0;

    if(nchan == 0 || nap == 0)
    {
        msg_error("fringex4", "empty phasor array" << eom);
        return {};
    }

    auto* ch_ax = &(std::get< 0 >(phasors)); // channel sky freqs (MHz); last entry = 0.0
    auto* ap_ax = &(std::get< 1 >(phasors)); // AP start times (s from scan start)

    double scan_duration = nap * ap_period; // total scan length in seconds

    // --- Segment list ---
    if(nsecs <= 0.0 || nsecs >= scan_duration)
        nsecs = scan_duration; // full-scan mode

    std::vector< double > seg_starts;
    for(double t = 0.0; t < scan_duration - 0.5 * ap_period; t += nsecs)
    {
        seg_starts.push_back(t);
    }
    if(seg_starts.empty())
    {
        seg_starts.push_back(0.0);
    }

    //OMODE (doubles the number of segments)
    if(overlap)
    {
        double half = nsecs / 2.0;
        std::vector< double > extra;
        for(double st : seg_starts)
        {
            double shifted = st - half; //OMODE starts segments half-a-chunk before the start of the scan
            if(shifted < scan_duration - 0.5 * ap_period)
            {
                extra.push_back(shifted);
            }
        }
        seg_starts.insert(seg_starts.end(), extra.begin(), extra.end());
        std::sort(seg_starts.begin(), seg_starts.end()); //fix up the order
        //add the last overlap segment if we can
        double shifted = seg_starts.back() + half;
        if(shifted < scan_duration - 0.5 * ap_period)
        {
            seg_starts.push_back(shifted);
        }
    }

    // --- Segmentation loop ---
    std::vector< SegmentResult > results;
    results.reserve(seg_starts.size());

    for(double seg_start : seg_starts)
    {
        double seg_end = seg_start + nsecs;

        double rsum = 0.0;
        double isum = 0.0;
        double ap_time_sum = 0.0; // sum of AP center times (s from scan start)
        int seg_nap = 0;

        for(std::size_t ap = 0; ap < nap; ap++)
        {
            double ap_start = ap_ax->at(ap);
            double ap_center = ap_start + ap_period / 2.0;

            if(ap_center < seg_start || ap_center >= seg_end)
                continue; // AP not in this segment

            // Time from FRT to AP center (used for fringe rotation)
            double tdelta = ap_center - frt_offset;

            // Sum over real channels with equal weights, applying delta correction
            double ch_rsum = 0.0;
            double ch_isum = 0.0;
            int valid = 0;

            for(std::size_t ch = 0; ch < nchan; ch++)
            {
                double freq = ch_ax->at(ch);
                if(freq == 0.0)
                    continue; // skip invalid channel entries

                std::complex< double > z = phasors(ch, ap);

                if(eff_dr != 0.0 || eff_mbd != 0.0)
                {
                    // Additional phase correction (cycles):
                    //   theta = freq*eff_dr*tdelta + eff_mbd*(freq - ref_freq)
                    double theta = freq * eff_dr * tdelta + eff_mbd * (freq - ref_freq);
                    double angle = -2.0 * M_PI * theta;
                    z *= std::complex< double >(std::cos(angle), std::sin(angle));
                }

                ch_rsum += z.real();
                ch_isum += z.imag();
                valid++;
            }

            if(valid > 0)
            {
                // Average across channels (equal weights)
                rsum += ch_rsum / valid;
                isum += ch_isum / valid;
                ap_time_sum += ap_center;
                seg_nap++;
            }
        }

        if(seg_nap == 0)
            continue; // empty segment - skip

        // --- Per-segment statistics ---

        // Normalize by number of accumulated APs -> amplitude per AP
        double amp = std::sqrt(rsum * rsum + isum * isum) / seg_nap;
        double resid_phas = std::atan2(isum, rsum) * 180.0 / M_PI; // degrees, wrap to [0, 360)
        if(resid_phas < 0.0)
            resid_phas += 360.0;

        // SNR: scale by sqrt(seg_nap/nap) for integration time, then by seg_amp/fourfit_amp
        // to account for amplitude variation across segments (mirrors fringex calc_seg.c:
        //   snr = t208->snr * sqrt(seglen[seg] / numaccp) * amp / tamp)
        // Note: use nap (AP count) not total_summed_weights (which sums over all axes)
        double snr = 0.0;
        if(nap > 0 && fourfit_amp > 0.0)
        {
            snr = full_snr * std::sqrt(static_cast< double >(seg_nap) / static_cast< double >(nap)) * (amp / fourfit_amp);
        }

        // Total phase = residual phase + linear delay/rate phase at segment centre
        // epochoff = time from FRT to segment centre
        double seg_center = seg_start + nsecs / 2.0;
        double epochoff = seg_center - frt_offset; // seconds

        // 360 * ref_freq [MHz] * delay [us] = 360 * 10^6 * 10^{-6} * cycles = 360 degrees/cycle

        // Mirrors fringex: TPHAS = rphase + bl_phase where bl_phase is the a-priori geometric
        // model phase (correlator splines).  Net: raw_phase - fourfit_fit.
        // Use quadratic Taylor expansion from spline model when available:
        //   delay(epochoff) = dm.delay + dm.rate*epochoff + 0.5*dm.accel*epochoff^2
        // Fall back to linear (a-priori = total - fit) when spline data is unavailable.
        double apriori_delay_at_epoch;
        if(dm.valid)
        {
            apriori_delay_at_epoch = dm.delay + dm.rate * epochoff + 0.5 * dm.accel * epochoff * epochoff;
        }
        else
        {
            double apriori_mbdelay = eff_total_mbdelay - fit_mbdelay;
            double apriori_drate = eff_total_drate - fit_drate;
            apriori_delay_at_epoch = apriori_mbdelay + apriori_drate * epochoff;
        }
        double total_phas = resid_phas + 360.0 * ref_freq * apriori_delay_at_epoch;

        // Wrap to [0, 360) matching fringex convention
        total_phas = std::fmod(total_phas, 360.0);
        if(total_phas < 0.0)
        {
            total_phas += 360.0;
        }

        // AP-weighted mean time offset from nominal midpoint (mirroring fringex calc_seg.c)
        // = round(mean AP center time) - nominal segment midpoint, in whole seconds
        double mean_ap_time = ap_time_sum / seg_nap;
        int offset_sec = static_cast< int >(std::round(mean_ap_time)) - static_cast< int >(std::round(seg_center));

        SegmentResult seg;
        seg.time_tag_sec = seg_center;
        if(overlap)
        {
            seg.time_tag_sec -= nsecs / 2.0;
        }
        seg.amp = amp;
        seg.resid_phas = resid_phas;
        seg.total_phas = total_phas;
        seg.snr = snr;
        seg.nap = seg_nap;
        seg.duration_sec = static_cast< int >(std::round(nsecs));
        seg.start_sec = static_cast< int >(std::round(seg_start));
        seg.offset_sec = offset_sec;
        seg.eff_total_mbdelay = eff_total_mbdelay;
        seg.eff_total_drate = eff_total_drate;
        seg.resid_mbdelay = resid_mbdelay;
        seg.resid_drate = resid_drate;
        seg.resid_delay = file_resid_delay + eff_mbd;
        results.push_back(seg);
    }

    return results;
}

// ---------------------------------------------------------------------------
// main
// ---------------------------------------------------------------------------
int main(int argc, char** argv)
{
    MHO_Message::GetInstance().AcceptAllKeys();

    // --- Command-line parsing ---
    CLI::App app{"fringex4 -- HOPS4 fringe re-segmentation tool\n"
                 "Reads .frng files and coherently re-segments the fringe data\n"
                 "into user-specified time intervals with optional delay/rate corrections."};

    std::vector< std::string > input_files;
    double seg_nsecs = 0.0;    // <=0 -> full scan
    double rate_offset = 0.0;  // us/s
    double delay_offset = 0.0; // us
    bool cmode = false;
    bool overlap = false;
    int message_level = 1; // message level: <0 debug, 0 verbose, 1 normal, 2+ quiet
    int alist_version = 6;
    std::string output_file = "-"; // "-" -> stdout

    app.add_option("input_files", input_files, "Input .frng file(s)")->required();
    app.add_option("-i,--duration", seg_nsecs, "Segment duration in seconds (0 or omit = full scan)");
    app.add_option("-r,--rate", rate_offset, "Delay rate offset in us/s (default 0; absolute value with -c)");
    app.add_option("-D,--delay", delay_offset, "MBD offset in us (default 0; absolute value with -c)");
    app.add_flag("-c,--cmode", cmode, "Absolute mode: -r/-D give total delay rate/MBD, not offsets");
    app.add_flag("-o,--overlap", overlap, "Overlap mode: produce double segments with half-segment offset");
    app.add_option("-m,--message_level", message_level, "Message level: -3..3, lower = more verbose (default 1)");
    app.add_option("-v,--alist-version", alist_version, "the a-file output version: 5 or 6 (default 6)")
        ->check(CLI::IsMember({5, 6})); //We only support version 5 & 6 output
    app.add_option("-O,--output", output_file, "Output file path (default: stdout)");

    CLI11_PARSE(app, argc, argv);

    //clamp message level
    if(message_level > 5)
    {
        message_level = 5;
    }
    if(message_level < -2)
    {
        message_level = -2;
    }
    MHO_Message::GetInstance().AcceptAllKeys();
    MHO_Message::GetInstance().SetLegacyMessageLevel(message_level); //use legacy style (lower value is more verbose)

    // --- Output stream, default is to dump to stdout ---
    std::ofstream out_file_stream;
    std::ostream* out = &std::cout;
    if(output_file != "-")
    {
        out_file_stream.open(output_file);
        if(!out_file_stream.good())
        {
            msg_fatal("fringex4", "cannot open output file: " << output_file << eom);
            return 1;
        }
        out = &out_file_stream;
    }

    // --- A-file header ---
    MHO_AFileInfoExtractor extractor;
    *out << extractor.GetAlistHeader(alist_version, 2, '*');

    // --- Current time for procdate ---
    std::string procdate_vex = hops_clock::to_vex_format(hops_clock::now(), true);

    // --- Process each input file ---
    int total_segments = 0;

    for(const std::string& fname : input_files)
    {
        msg_info("fringex4", "processing: " << fname << eom);

        mho_json base_fsum;
        phasor_type phasors;
        MHO_ParameterStore params;

        if(!read_frng_file(fname, base_fsum, phasors, params))
        {
            msg_error("fringex4", "skipping file (read failed): " << fname << eom);
            continue;
        }

        // Scan start time for converting relative times to absolute VEX strings
        std::string scan_start_vex;
        if(!params.Get("/fringe/start_date", scan_start_vex))
        {
            msg_error("fringex4", "missing /fringe/start_date in: " << fname << eom);
            continue;
        }
        hops_clock::time_point scan_start_tp = hops_clock::from_vex_format(scan_start_vex);

        // Load spline-based a-priori delay model for accurate TPHAS (quadratic correction)
        AprioriDelayModel dm = compute_apriori_delay_model(fname, params);

        // Compute segments
        auto segments = compute_segments(phasors, params, seg_nsecs, rate_offset, delay_offset, cmode, overlap, dm);

        msg_info("fringex4", "  -> " << segments.size() << " segment(s)" << eom);

        // Output one A-file record per segment
        for(const SegmentResult& seg : segments)
        {
            // Absolute time of segment centre
            auto seg_tp = scan_start_tp + std::chrono::duration_cast< std::chrono::nanoseconds >(
                                              std::chrono::duration< double >(seg.time_tag_sec));
            std::string seg_time_tag = hops_clock::to_vex_format(seg_tp, true);

            // Copy baseline fields from the fringe-file summary and override
            // the per-segment quantities
            mho_json row = base_fsum;

            // Identity / position in scan
            // extent_no: keep file-derived value from base_fsum (not per-segment counter)
            row["time_tag"] = seg_time_tag;
            row["epoch"] = seg_time_tag; // EPCH = segment centre (mm:ss)
            row["procdate"] = procdate_vex;

            // Segment timing
            row["duration"] = static_cast< int64_t >(seg.duration_sec);
            row["length"] = seg.nap;                                   // APs accumulated
            row["offset"] = seg.offset_sec;                            // AP mean time minus nominal midpoint
            row["scan_offset"] = seg.start_sec + seg.duration_sec / 2; // segment midpoint

            // Fringe quantities for this segment
            row["amp"] = seg.amp;
            row["snr"] = seg.snr;
            row["phase_snr"] = seg.snr;
            row["resid_phas"] = seg.resid_phas;
            row["total_phas"] = seg.total_phas;

            // Delay / rate - residual part stored in mbdelay/delay_rate;
            // total (a-priori + residual + user offset) in total_mbdelay/total_rate
            row["mbdelay"] = seg.resid_mbdelay;  // residual + user offset (us)
            row["delay_rate"] = seg.resid_drate; // residual rate + user offset (us/s)
            row["total_mbdelay"] = seg.eff_total_mbdelay;
            row["total_rate"] = seg.eff_total_drate;
            row["resid_delay"] = seg.resid_delay; // v6 RESIDUALDELAY column: N*AMB + resid_mbdelay to match SBD

            // datatype: 'C'=coherent, 'O'=overlap mode; always 'f' suffix (mirroring fringex)
            row["datatype"] = overlap ? "Of" : "Cf";

            // coherence times: not computed by fringex4, set to 0
            row["srch_cotime"] = 0;
            row["noloss_cotime"] = 0;

            *out << extractor.ConvertToAlistRow(row, alist_version);
            total_segments++;
        }

        *out << "*endofscan\n";
    }

    msg_info("fringex4", "total segments output: " << total_segments << eom);
    return 0;
}
