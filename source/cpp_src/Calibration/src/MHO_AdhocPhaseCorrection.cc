#include "MHO_AdhocPhaseCorrection.hh"

#include <cmath>
#include <fstream>
#include <sstream>

namespace hops
{

MHO_AdhocPhaseCorrection::MHO_AdhocPhaseCorrection()
{
    fMode = AdhocPhaseMode::NONE;

    fTRef      = 0.0;
    fPeriod    = 1.0;
    fAmplitude = 0.0;
    for(int i = 0; i < 6; i++){fPolyCoeffs[i] = 0.0;}
        

    for(std::size_t stn = 0; stn < 2; stn++)
    {
        fAhFile[stn]      = "";
        fAhFileChans[stn] = "";
        fNFileRows[stn]   = 0;
        fNFileCols[stn]   = 0;
    }

    fScanStartFpDay = 0.0;
    fScanStartSecPastHour = 0.0;
    fScanYear       = 0;
    fAccPeriod      = 0.0;

    fImagUnit       = MHO_Constants::imag_unit;
    fChannelLabelKey = "channel_label";
    fStartKey        = "start";
}

MHO_AdhocPhaseCorrection::~MHO_AdhocPhaseCorrection() {}

// ---------------------------------------------------------------------------
// Setters for polynomial coefficients
// ---------------------------------------------------------------------------

void MHO_AdhocPhaseCorrection::SetPolynomialCoeffs(const std::vector< double >& coeffs)
{
    std::cout<<"setting polys"<<std::endl;
    for(int i = 0; i < 6; i++){fPolyCoeffs[i] = 0.0;}
    std::size_t n = std::min(coeffs.size(), (size_t)6);
    for(std::size_t i = 0; i < n; i++)
    {
        fPolyCoeffs[i] = coeffs[i];
        std::cout<<"poly coeff @ "<<i<<" = "<<fPolyCoeffs[i]<<std::endl;
    }
}

// ---------------------------------------------------------------------------
// Setters / getters for PHYLE file names
// ---------------------------------------------------------------------------

void MHO_AdhocPhaseCorrection::SetRefAdhocFile(const std::string& filename, const std::string& chans)
{
    fAhFile[0]      = filename;
    fAhFileChans[0] = chans;
}

void MHO_AdhocPhaseCorrection::GetRefAdhocFile(std::string& filename, std::string& chans) const
{
    filename = fAhFile[0];
    chans    = fAhFileChans[0];
}

void MHO_AdhocPhaseCorrection::SetRemAdhocFile(const std::string& filename, const std::string& chans)
{
    fAhFile[1]      = filename;
    fAhFileChans[1] = chans;
}

void MHO_AdhocPhaseCorrection::GetRemAdhocFile(std::string& filename, std::string& chans) const
{
    filename = fAhFile[1];
    chans    = fAhFileChans[1];
}

bool MHO_AdhocPhaseCorrection::InitializeInPlace(visibility_type* in)
{
    if(fMode == AdhocPhaseMode::NONE)
    {
        //do nothing
        msg_debug("calibration", "no adhoc_phase mode selected, will not modify visibilities" << eom);
        return true;
    }

    // extract scan start time from visibility metadata,
    // and convert it to floating-point days since the start of the year 
    // to match the legacy time convention used for adhoc_phase application
    std::string vis_start;
    bool ok = in->Retrieve(fStartKey, vis_start);
    if(!ok)
    {
        msg_error("calibration", "MHO_AdhocPhaseCorrection: could not retrieve <start> tag from visibility object." << eom);
        return false;
    }

    hops_clock::time_point scan_start_tp = hops_clock::from_vex_format(vis_start);
    hops_clock::to_year_fpday(scan_start_tp, fScanYear, fScanStartFpDay);

    msg_debug("calibration", "MHO_AdhocPhaseCorrection: scan start = " << vis_start
                             << "  year =" << fScanYear
                             << "  floating point day =" << fScanStartFpDay << eom);

    //now we need to calculate the scan start as seconds past the hour 
    //this is needed for poly and sinewave models
    auto last_hour = hops_clock::to_legacy_hops_date(scan_start_tp);
    //zero out the mins/secs, then convert back and compute the difference
    last_hour.minute = 0;
    last_hour.second = 0;
    auto last_hour_tp =  hops_clock::from_legacy_hops_date(last_hour);
    auto tdiff_duration = scan_start_tp - last_hour_tp;
    fScanStartSecPastHour = std::chrono::duration< double >(tdiff_duration).count();

    //derive accumulation-period duration from the time axis 
    auto* time_ax = &(std::get< TIME_AXIS >(*in));
    std::size_t nap = time_ax->GetSize();
    if(nap < 2)
    {
        //single AP, can't determin AP, this is nonsense, so abort
        msg_warn("calibration", "MHO_AdhocPhaseCorrection: can't determine AP size" << eom);
        return false;
    }
    else
    {
        fAccPeriod = time_ax->at(1) - time_ax->at(0);
    }

    // -- For PHYLE mode, load both station files ---------------------------
    if(fMode == AdhocPhaseMode::PHYLE)
    {
        bool ref_ok = LoadAdhocFile(0);
        bool rem_ok = LoadAdhocFile(1);
        if(!ref_ok || !rem_ok)
        {
            if(!ref_ok){msg_error("calibration", "MHO_AdhocPhaseCorrection: failed to load reference station adhoc phase file" << eom);}
            if(!rem_ok){msg_error("calibration", "MHO_AdhocPhaseCorrection: failed to load remote station adhoc phase file" << eom);}
            return false;
        }
    }

    return true;
}

bool MHO_AdhocPhaseCorrection::InitializeOutOfPlace(const visibility_type* in, visibility_type* out)
{
    out->Copy(*in);
    return InitializeInPlace(out);
}

bool MHO_AdhocPhaseCorrection::LoadAdhocFile(std::size_t stn_idx)
{
    fFileData[stn_idx].clear();
    fNFileRows[stn_idx] = 0;
    fNFileCols[stn_idx] = 0;

    // An empty filename means "not used" , that is legal.
    if(fAhFile[stn_idx].empty()){return true;}

    std::size_t nchan = fAhFileChans[stn_idx].size();
    if(nchan == 0)
    {
        msg_error("calibration", "MHO_AdhocPhaseCorrection: adhoc file set for station "
                  << stn_idx << " but channel string is empty." << eom);
        return false;
    }

    std::ifstream fs(fAhFile[stn_idx]);
    if(!fs.is_open())
    {
        msg_error("calibration", "MHO_AdhocPhaseCorrection: cannot open adhoc file: "
                  << fAhFile[stn_idx] << eom);
        return false;
    }

    msg_debug("calibration", "MHO_AdhocPhaseCorrection: reading adhoc file for station "
              << stn_idx << ": " << fAhFile[stn_idx]
              << "  channels: " << fAhFileChans[stn_idx] << eom);

    // Each data row has (nchan + 1) doubles: [t_fpday, phase_0, ..., phase_N].
    // Comment lines (starting with '#' or any non-digit first token) are skipped.
    std::size_t ncols = nchan + 1;
    std::string line;
    while(std::getline(fs, line))
    {
        // Skip blank lines and lines whose first non-space character is not a digit or '.'
        std::size_t first_nonspace = line.find_first_not_of(" \t\r\n");
        if(first_nonspace == std::string::npos){continue;}
        char first_char = line[first_nonspace];
        if(!std::isdigit(static_cast< unsigned char >(first_char)) && first_char != '.'){continue;}

        std::istringstream iss(line);
        std::vector< double > row(ncols);
        bool parse_ok = true;
        for(std::size_t col = 0; col < ncols; col++)
        {
            if(!(iss >> row[col]))
            {
                msg_warn("calibration", "MHO_AdhocPhaseCorrection: short or malformed line in "
                         << fAhFile[stn_idx] << "; line skipped." << eom);
                parse_ok = false;
                break;
            }
        }
        if(!parse_ok){continue;}

        // Append row values to flat storage
        for(std::size_t col = 0; col < ncols; col++)
        {
            fFileData[stn_idx].push_back(row[col]);
        }
        fNFileRows[stn_idx]++;
    }

    if(fNFileRows[stn_idx] == 0)
    {
        msg_warn("calibration", "MHO_AdhocPhaseCorrection: no data rows found in " << fAhFile[stn_idx] << eom);
        return true; // non-fatal; InterpolateFilePhase returns 0
    }

    // Mirror the legacy edge case: if there is only one data row, duplicate it
    // so that the interpolation interval is well-defined.
    if(fNFileRows[stn_idx] == 1)
    {
        for(std::size_t col = 0; col < ncols; col++)
        {
            fFileData[stn_idx].push_back(fFileData[stn_idx][col]);
        }
        fNFileRows[stn_idx] = 2;
    }
    fNFileCols[stn_idx] = ncols;

    msg_debug("calibration", "MHO_AdhocPhaseCorrection: loaded " << fNFileRows[stn_idx] << " rows from " << fAhFile[stn_idx] << eom);
    return true;
}

bool MHO_AdhocPhaseCorrection::ExecuteInPlace(visibility_type* in)
{
    if(fMode == AdhocPhaseMode::NONE)
        return true;

    auto* pp_ax   = &(std::get< POLPROD_AXIS >(*in));
    auto* chan_ax  = &(std::get< CHANNEL_AXIS >(*in));
    auto* time_ax  = &(std::get< TIME_AXIS >(*in));

    std::size_t npp   = pp_ax->GetSize();
    std::size_t nch   = chan_ax->GetSize();
    std::size_t nap   = time_ax->GetSize();
    std::size_t nfreq = in->GetDimension(FREQ_AXIS);

    for(std::size_t ch = 0; ch < nch; ch++)
    {
        // Retrieve the fourfit channel label (freq-code character) for this channel.
        std::string chan_label;
        bool has_label = chan_ax->RetrieveIndexLabelKeyValue(ch, fChannelLabelKey, chan_label);
        if(!has_label)
        {
            msg_warn("calibration", "MHO_AdhocPhaseCorrection: channel " << ch
                     << " has no channel_label; skipping." << eom);
            continue;
        }

        for(std::size_t t = 0; t < nap; t++)
        {
            //AP centre time in seconds from scan start
            double ap_center_sec = time_ax->at(t) + 0.5 * fAccPeriod;
            //compute the phase correction to be applied
            double zeta = ComputeZeta(chan_label, ap_center_sec);
            //Apply exp(-i*zeta) to all pol-products and spectral points at (ch, t)
            std::complex< double > phasor = std::exp(-fImagUnit * zeta);
            for(std::size_t pp = 0; pp < npp; pp++)
            {
                for(std::size_t f = 0; f < nfreq; f++)
                {
                    (*in)(pp, ch, t, f) *= phasor;
                }
            }
        }
    }

    return true;
}

bool MHO_AdhocPhaseCorrection::ExecuteOutOfPlace(const visibility_type* in, visibility_type* out)
{
    out->Copy(*in);
    return ExecuteInPlace(out);
}

// ---------------------------------------------------------------------------
// ComputeZeta
// ---------------------------------------------------------------------------

double MHO_AdhocPhaseCorrection::ComputeZeta(const std::string& chan_label, double ap_center_sec) const
{
    double zeta = 0.0;
    
    //legacy time calculation:
    //thyme = (ap + 0.5) * param.acc_period + param.start - param.ah_tref;
    double thyme = ap_center_sec + (fScanStartSecPastHour - fTRef);

    switch(fMode)
    {
        case AdhocPhaseMode::SINEWAVE:
        {
            double phase_arg = 2.0 * M_PI * thyme / fPeriod;
            zeta = fAmplitude * std::sin(phase_arg);
            break;
        }

        case AdhocPhaseMode::POLYNOMIAL:
        {
            //evaluate zeta = c0 + c1*t + c2*t^2 + ...
            //nothing fancy, we are not using Horner's method
            double thyme_n = 1.0;
            for(int i = 0; i < 6; i++)
            {
                zeta += fPolyCoeffs[i] * thyme_n;
                thyme_n *= thyme;
            }
            //std::cout<<"zeta = "<<zeta<<std::endl;
            break;
        }

        case AdhocPhaseMode::PHYLE:
        {
            // Convert AP centre time (seconds from scan start) to
            // fractional days since beginning of year, then interpolate the files.
            double ap_center_fpday = fScanStartFpDay + ap_center_sec / 86400.0;
            char fcode = chan_label.empty() ? '\0' : chan_label[0];
            double phase_ref = InterpolateFilePhase(0, fcode, ap_center_fpday);
            double phase_rem = InterpolateFilePhase(1, fcode, ap_center_fpday);
            // Differential correction (ref - rem), already in radians
            zeta = phase_ref - phase_rem;
            break;
        }

        default:
            zeta = 0.0;
            break;
    }

    return zeta;
}

double MHO_AdhocPhaseCorrection::InterpolateFilePhase(std::size_t stn_idx, char fcode, double t_fpday) const
{
    // No file for this station , no correction.
    if(fAhFile[stn_idx].empty() || fNFileRows[stn_idx] == 0){return 0.0;}

    // Find the column index for this freq-code in the channel string.
    std::size_t nch_col = fAhFileChans[stn_idx].find(fcode);
    if(nch_col == std::string::npos)
    {
        msg_warn("calibration", "MHO_AdhocPhaseCorrection: freq code '"
                 << fcode << "' not found in channel string '"
                 << fAhFileChans[stn_idx] << "' for station " << stn_idx
                 << "; returning 0." << eom);
        return 0.0;
    }

    const std::vector< double >& data = fFileData[stn_idx];
    std::size_t ncols = fNFileCols[stn_idx];
    std::size_t nrows = fNFileRows[stn_idx];

    // Helper lambda: time value for row r
    auto row_time = [&](std::size_t r) -> double { return data[r * ncols]; };
    // Helper lambda: phase value (in degrees) for row r at the requested channel column
    auto row_phase = [&](std::size_t r) -> double { return data[r * ncols + nch_col + 1]; };

    // Clamp the requested time to the span covered by the file data.
    double t_first = row_time(0);
    double t_last  = row_time(nrows - 1);
    double t_bound = t_fpday;
    if(t_bound < t_first)
    {
        t_bound = t_first;
    }
    else if(t_bound > t_last)
    {
        t_bound = t_last;
    }

    // Find the bounding interval [n-1, n] by linear scan (file sizes are small).
    std::size_t n = 1;
    while(n < nrows - 1 && row_time(n) < t_bound){n++;}

    // Linear interpolation between rows n-1 and n.
    double t_a   = row_time(n - 1);
    double t_b   = row_time(n);
    double phi_a = row_phase(n - 1);
    double phi_b = row_phase(n);

    double phase_deg = (t_bound * (phi_b - phi_a) - t_a * phi_b + t_b * phi_a) / (t_b - t_a);

    // Convert degrees to radians before returning.
    return phase_deg * MHO_Constants::deg_to_rad; // M_PI / 180.0;
}

} // namespace hops
