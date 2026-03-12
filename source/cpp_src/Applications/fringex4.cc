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

#include "MHO_Message.hh"
#include "MHO_Clock.hh"

#include "MHO_BinaryFileInterface.hh"
#include "MHO_ContainerDefinitions.hh"
#include "MHO_ContainerDictionary.hh"
#include "MHO_ObjectTags.hh"
#include "MHO_ParameterStore.hh"

#include "MHO_AFileInfoExtractor.hh"

using namespace hops;

// ---------------------------------------------------------------------------
// Per-segment result
// ---------------------------------------------------------------------------
struct SegmentResult
{
    double time_tag_sec;  // seconds from scan start to segment center
    double amp;           // amplitude (same units as /fringe/famp)
    double resid_phas;    // residual phase (degrees, -180..+180)
    double total_phas;    // total phase (degrees)
    double snr;           // SNR estimate
    int    nap;           // number of APs accumulated
    int    duration_sec;  // segment duration in whole seconds
    int    start_sec;     // segment start offset from scan start (seconds)
    // effective total delay/rate used for phase calculation
    double eff_total_mbdelay; // us
    double eff_total_drate;   // us/s
};

// ---------------------------------------------------------------------------
// Read phasor data and parameters from a .frng binary file
// Returns false on failure.
// ---------------------------------------------------------------------------
static bool read_frng_file(const std::string& filename,
                            mho_json&           base_fsum,
                            phasor_type&        phasors,
                            MHO_ParameterStore& params)
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
    std::vector< MHO_FileKey >   keys;
    std::vector< std::size_t >   offsets;
    if(!inter.ExtractFileObjectKeysAndOffsets(filename, keys, offsets))
    {
        msg_error("fringex4", "failed to read object keys from: " << filename << eom);
        return false;
    }

    // --- Find and read MHO_ObjectTags (by type UUID) to populate params ---
    MHO_ContainerDictionary cdict;
    MHO_UUID tag_uuid = cdict.GetUUIDFor< MHO_ObjectTags >();

    bool   found_tags  = false;
    std::size_t tags_offset = 0;
    for(std::size_t i = 0; i < keys.size(); i++)
    {
        if(keys[i].fTypeId == tag_uuid)
        {
            tags_offset   = offsets[i];
            found_tags    = true;
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
    MHO_FileKey    key;
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
    bool   found_phasors   = false;
    std::size_t phasors_offset = 0;
    for(std::size_t i = 0; i < keys.size(); i++)
    {
        std::string kname(keys[i].fName);
        if(kname == "phasors")
        {
            phasors_offset = offsets[i];
            found_phasors  = true;
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
static std::vector< SegmentResult >
compute_segments(const phasor_type&        phasors,
                 const MHO_ParameterStore& params,
                 double                    nsecs,        // segment length (s); <=0 -> full scan
                 double                    user_drate,   // us/s: offset or absolute (cmode)
                 double                    user_mbdelay, // us:   offset or absolute (cmode)
                 bool                      cmode,        // true -> user values are absolute
                 bool                      overlap)      // true -> half-step interleaved windows
{
    // --- Parameters from file ---
    double ap_period     = params.GetAs< double >("/config/ap_period");          // s
    double frt_offset    = params.GetAs< double >("/config/frt_offset");         // s from scan start
    double ref_freq      = params.GetAs< double >("/control/config/ref_freq");   // MHz
    double fit_mbdelay   = params.GetAs< double >("/fringe/mbdelay");            // us (residual)
    double fit_drate     = params.GetAs< double >("/fringe/drate");              // us/s (residual)
    double total_mbdelay = params.GetAs< double >("/fringe/total_mbdelay");      // us (a-priori)
    double total_drate   = params.GetAs< double >("/fringe/total_drate");        // us/s (a-priori)
    double full_snr      = params.GetAs< double >("/fringe/snr");
    double total_weight  = params.GetAs< double >("/fringe/total_summed_weights");

    // Total applied delay and rate (what has been removed from the phasors)
    double applied_mbdelay = total_mbdelay + fit_mbdelay; // us
    double applied_drate   = total_drate   + fit_drate;   // us/s

    // Effective delta correction to apply to the already-stopped phasors
    double eff_dr, eff_mbd;
    if(cmode)
    {
        // user values are absolute total; delta = user - applied
        eff_dr  = user_drate   - applied_drate;
        eff_mbd = user_mbdelay - applied_mbdelay;
    }
    else
    {
        // user values are additional offsets
        eff_dr  = user_drate;
        eff_mbd = user_mbdelay;
    }

    // Effective total delay and rate to report in A-file
    double eff_total_drate   = applied_drate   + eff_dr;
    double eff_total_mbdelay = applied_mbdelay + eff_mbd;

    // --- Phasor dimensions ---
    std::size_t nchan_plus1 = phasors.GetDimension(0); // nchan real channels + 1 "All"
    std::size_t nap         = phasors.GetDimension(1);
    std::size_t nchan       = nchan_plus1 > 0 ? nchan_plus1 - 1 : 0;

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
        seg_starts.push_back(t);
    if(seg_starts.empty())
        seg_starts.push_back(0.0);

    if(overlap)
    {
        double half = nsecs / 2.0;
        std::vector< double > extra;
        for(double st : seg_starts)
        {
            double shifted = st + half;
            if(shifted < scan_duration - 0.5 * ap_period)
                extra.push_back(shifted);
        }
        seg_starts.insert(seg_starts.end(), extra.begin(), extra.end());
        std::sort(seg_starts.begin(), seg_starts.end());
    }

    // --- Segmentation loop ---
    std::vector< SegmentResult > results;
    results.reserve(seg_starts.size());

    for(double seg_start : seg_starts)
    {
        double seg_end = seg_start + nsecs;

        double rsum    = 0.0;
        double isum    = 0.0;
        int    seg_nap = 0;

        for(std::size_t ap = 0; ap < nap; ap++)
        {
            double ap_start  = ap_ax->at(ap);
            double ap_center = ap_start + ap_period / 2.0;

            if(ap_center < seg_start || ap_center >= seg_end)
                continue; // AP not in this segment

            // Time from FRT to AP center (used for fringe rotation)
            double tdelta = ap_center - frt_offset;

            // Sum over real channels with equal weights, applying delta correction
            double ch_rsum = 0.0;
            double ch_isum = 0.0;
            int    valid   = 0;

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
                seg_nap++;
            }
        }

        if(seg_nap == 0)
            continue; // empty segment - skip

        // --- Per-segment statistics ---

        // Normalize by number of accumulated APs -> amplitude per AP
        double amp       = std::sqrt(rsum * rsum + isum * isum) / seg_nap;
        double resid_phas = std::atan2(isum, rsum) * 180.0 / M_PI; // degrees

        // SNR scales as sqrt of integration time (equal-weight assumption)
        double snr = 0.0;
        if(total_weight > 0.0)
            snr = full_snr * std::sqrt(static_cast< double >(seg_nap) / total_weight);

        // Total phase = residual phase + linear delay/rate phase at segment centre
        // epochoff = time from FRT to segment centre
        double seg_center = seg_start + nsecs / 2.0;
        double epochoff   = seg_center - frt_offset; // seconds

        // 360 * ref_freq [MHz] * delay [us] = 360 * 10^6 * 10^{-6} * cycles = 360 degrees/cycle 
        double total_phas = resid_phas
                            + 360.0 * ref_freq
                                  * (eff_total_mbdelay + eff_total_drate * epochoff);
        // Wrap to (-180, +180]
        total_phas = std::fmod(total_phas, 360.0);
        if(total_phas >  180.0) total_phas -= 360.0;
        if(total_phas < -180.0) total_phas += 360.0;

        SegmentResult seg;
        seg.time_tag_sec      = seg_center;
        seg.amp               = amp;
        seg.resid_phas        = resid_phas;
        seg.total_phas        = total_phas;
        seg.snr               = snr;
        seg.nap               = seg_nap;
        seg.duration_sec      = static_cast< int >(std::round(nsecs));
        seg.start_sec         = static_cast< int >(std::round(seg_start));
        seg.eff_total_mbdelay = eff_total_mbdelay;
        seg.eff_total_drate   = eff_total_drate;
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
    double      seg_nsecs     = 0.0;  // <=0 -> full scan
    double      rate_offset   = 0.0;  // us/s
    double      delay_offset  = 0.0;  // us
    bool        cmode         = false;
    bool        overlap       = false;
    bool        quiet         = false;
    bool        verbose       = false;
    int         alist_version = 6;
    std::string output_file   = "-";  // "-" -> stdout

    app.add_option("input_files,-i,--input", input_files,
                   "Input .frng file(s)")
        ->required();
    app.add_option("-d,--duration", seg_nsecs,
                   "Segment duration in seconds (0 or omit = full scan)");
    app.add_option("-r,--rate", rate_offset,
                   "Delay rate offset in us/s (default 0; absolute value with -c)");
    app.add_option("-m,--delay", delay_offset,
                   "MBD offset in us (default 0; absolute value with -c)");
    app.add_flag("-c,--cmode", cmode,
                 "Absolute mode: -r/-m give total delay rate/MBD, not offsets");
    app.add_flag("-o,--overlap", overlap,
                 "Overlap mode: produce double segments with half-segment offset");
    app.add_flag("-q,--quiet", quiet,
                 "Suppress informational messages");
    app.add_flag("-v,--verbose", verbose,
                 "Enable verbose/debug output");
    app.add_option("-V,--alist-version", alist_version,
                   "A-file output version: 5 or 6 (default 6)")
        ->check(CLI::IsMember({5, 6}));
    app.add_option("-O,--output", output_file,
                   "Output file path (default: stdout)");

    CLI11_PARSE(app, argc, argv);

    // --- Verbosity ---
    if(quiet)
        MHO_Message::GetInstance().SetMessageLevel(eWarning);
    else if(verbose)
        MHO_Message::GetInstance().SetMessageLevel(eDebug);

    // --- Output stream ---
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

        mho_json           base_fsum;
        phasor_type        phasors;
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

        // Compute segments
        auto segments = compute_segments(phasors, params,
                                         seg_nsecs,
                                         rate_offset, delay_offset,
                                         cmode, overlap);

        msg_info("fringex4", "  -> " << segments.size() << " segment(s)" << eom);

        // Output one A-file record per segment
        int seg_idx = 1;
        for(const SegmentResult& seg : segments)
        {
            // Absolute time of segment centre
            auto seg_tp = scan_start_tp
                          + std::chrono::duration_cast< std::chrono::nanoseconds >(
                                std::chrono::duration< double >(seg.time_tag_sec));
            std::string seg_time_tag = hops_clock::to_vex_format(seg_tp, true);

            // Copy baseline fields from the fringe-file summary and override
            // the per-segment quantities
            mho_json row = base_fsum;

            // Identity / position in scan
            row["extent_no"]  = seg_idx;
            row["time_tag"]   = seg_time_tag;
            row["epoch"]      = seg_time_tag; // EPCH = segment centre (mm:ss)
            row["procdate"]   = procdate_vex;

            // Segment timing
            row["duration"]     = static_cast< int64_t >(seg.duration_sec);
            row["length"]       = seg.nap;       // APs accumulated
            row["offset"]       = 0;             // lag offset (not applicable)
            row["scan_offset"]  = seg.start_sec; // seconds from scan start

            // Fringe quantities for this segment
            row["amp"]          = seg.amp;
            row["snr"]          = seg.snr;
            row["phase_snr"]    = seg.snr;
            row["resid_phas"]   = seg.resid_phas;
            row["total_phas"]   = seg.total_phas;

            // Delay / rate - residual part stored in mbdelay/delay_rate;
            // total (a-priori + residual + user offset) in total_mbdelay/total_rate
            row["mbdelay"]        = seg.eff_total_mbdelay; // effective total MBD (us)
            row["delay_rate"]     = seg.eff_total_drate;   // effective total rate (us/s)
            row["total_mbdelay"]  = seg.eff_total_mbdelay;
            row["total_rate"]     = seg.eff_total_drate;
            row["resid_delay"]    = seg.eff_total_mbdelay; // v6 RESIDUALDELAY column

            *out << extractor.ConvertToAlistRow(row, alist_version);
            seg_idx++;
            total_segments++;
        }

        *out << "*endofscan\n";
    }

    msg_info("fringex4", "total segments output: " << total_segments << eom);
    return 0;
}
