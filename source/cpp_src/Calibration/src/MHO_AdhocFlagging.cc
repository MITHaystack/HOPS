#include "MHO_AdhocFlagging.hh"

#include <algorithm>
#include <cstring>
#include <fstream>
#include <sstream>

namespace hops
{

// ---------------------------------------------------------------------------
// Helpers with file scope
// ---------------------------------------------------------------------------

static bool AHF_IsComment(const std::string& line)
{
    std::size_t start = line.find_first_not_of(" \t");
    if(start == std::string::npos)
    {
        return true;
    } // blank / whitespace-only
    return (line[start] == '*');
}

// ---------------------------------------------------------------------------
// Constructor / destructor
// ---------------------------------------------------------------------------

MHO_AdhocFlagging::MHO_AdhocFlagging()
{
    fFlagFile[0] = "";
    fFlagFile[1] = "";
    fScanStartFpDay = 0.0;
    fScanYear = 0;
    fAccPeriod = 1.0;
    fStartKey = "start";
    fSidebandKey = "net_sideband";
    fParameterStore = nullptr;
}

MHO_AdhocFlagging::~MHO_AdhocFlagging()
{}

// ---------------------------------------------------------------------------
// DecodeHexToken (static)
// ---------------------------------------------------------------------------

/*static*/
void MHO_AdhocFlagging::DecodeHexToken(const char* buf, std::array< uint8_t, MAX_FLAG_FREQS >& bytes)
{
    bytes.fill(0);

    int limit = (int)strlen(buf);
    int offset = 0;
    int lastusb = 0;
    int lastlsb = 1; // safe: if buf has <2 chars the null byte yields nibble=0xF

    for(int addr = 0, usb = 1; addr < (int)MAX_FLAG_FREQS; usb = !usb)
    {
        int nibble;
        if(buf[offset])
            nibble = buf[offset];
        else if(usb)
            nibble = buf[lastusb];
        else
            nibble = buf[lastlsb];

        if(nibble >= '0' && nibble <= '9')
            nibble -= '0';
        else if(nibble >= 'A' && nibble <= 'F')
            nibble -= ('A' - 10);
        else if(nibble >= 'a' && nibble <= 'f')
            nibble -= ('a' - 10);
        else
            nibble = 0xF; // illegal -> retain all

        if(usb)
            bytes[addr] = static_cast< uint8_t >(nibble << 4);
        else
            bytes[addr++] |= static_cast< uint8_t >(nibble);

        if(buf[offset])
        {
            if(usb)
                lastusb = offset;
            else
                lastlsb = offset;
        }
        if(offset < limit)
            offset++;
    }
}

// ---------------------------------------------------------------------------
// LoadFlagFile
// ---------------------------------------------------------------------------

bool MHO_AdhocFlagging::LoadFlagFile(std::size_t stn_idx)
{
    fFlagTable[stn_idx].clear();

    // An empty filename means "not configured" - legal, station contributes 0xFF.
    if(fFlagFile[stn_idx].empty())
    {
        return true;
    }

    std::ifstream fs(fFlagFile[stn_idx]);
    if(!fs.is_open())
    {
        msg_error("calibration", "MHO_AdhocFlagging: cannot open flag file: " << fFlagFile[stn_idx] << eom);
        return false;
    }

    msg_debug("calibration", "MHO_AdhocFlagging: loading flag table [" << stn_idx << "] from " << fFlagFile[stn_idx] << eom);

    std::string line;
    std::size_t n_loaded = 0;
    while(std::getline(fs, line))
    {
        if(AHF_IsComment(line))
        {
            continue;
        }

        // Expect exactly two tokens: <fpday>  <hex_string>
        std::istringstream iss(line);
        double fpday;
        std::string hex_token;
        if(!(iss >> fpday >> hex_token))
        {
            msg_warn("calibration",
                     "MHO_AdhocFlagging: malformed line in " << fFlagFile[stn_idx] << " (skipped): " << line << eom);
            continue;
        }

        FlagTableRow row;
        row.time_fpday = fpday;
        DecodeHexToken(hex_token.c_str(), row.bytes);
        fFlagTable[stn_idx].push_back(std::move(row));
        n_loaded++;
    }

    msg_debug("calibration", "MHO_AdhocFlagging: loaded " << n_loaded << " rows from " << fFlagFile[stn_idx] << eom);

    if(n_loaded == 0)
    {
        msg_warn("calibration", "MHO_AdhocFlagging: no data rows found in " << fFlagFile[stn_idx] << eom);
    }

    return true;
}

// ---------------------------------------------------------------------------
// LookupFlagBytes
// ---------------------------------------------------------------------------

const uint8_t* MHO_AdhocFlagging::LookupFlagBytes(std::size_t stn_idx, double ap_center_fpday) const
{
    // Sentinel: all 0xFF means "retain everything" (no flagging).
    static const std::array< uint8_t, MAX_FLAG_FREQS > ALL_GOOD = []() {
        std::array< uint8_t, MAX_FLAG_FREQS > a;
        a.fill(0xFF);
        return a;
    }();

    const auto& table = fFlagTable[stn_idx];

    // Empty table or time out of file range -> no flagging (matches legacy locate_entry).
    if(table.empty())
    {
        return ALL_GOOD.data();
    }
    if(ap_center_fpday < table.front().time_fpday)
    {
        return ALL_GOOD.data();
    }
    if(ap_center_fpday > table.back().time_fpday)
    {
        return ALL_GOOD.data();
    }

    // Lower-bound lookup: find the last row whose time <= ap_center_fpday.
    // std::upper_bound returns iterator to first row with time > ap_center_fpday;
    // stepping back one gives the desired row.
    auto it = std::upper_bound(table.begin(), table.end(), ap_center_fpday,
                               [](double t, const FlagTableRow& row) { return t < row.time_fpday; });
    if(it != table.begin())
    {
        --it;
    }

    //std::cout << "found bytes: ";
    //for (auto b : it->bytes) //std::cout << std::hex << (int)b << " ";
    //std::cout << std::dec << std::endl;

    return it->bytes.data();
}

// ---------------------------------------------------------------------------
// InitializeInPlace / OutOfPlace
// ---------------------------------------------------------------------------

bool MHO_AdhocFlagging::InitializeInPlace(weight_type* in)
{
    // Both files empty -> nothing to do.
    if(fFlagFile[0].empty() && fFlagFile[1].empty())
    {
        msg_debug("calibration", "MHO_AdhocFlagging: no flag files configured, operator is a no-op." << eom);
        return true;
    }

    // Extract scan start time from container metadata.
    std::string start_str;
    if(!in->Retrieve(fStartKey, start_str))
    {
        msg_error("calibration", "MHO_AdhocFlagging: could not retrieve <start> tag from weight object." << eom);
        return false;
    }

    hops_clock::time_point scan_start_tp = hops_clock::from_vex_format(start_str);
    hops_clock::to_year_fpday(scan_start_tp, fScanYear, fScanStartFpDay);

    msg_debug("calibration", "MHO_AdhocFlagging: scan start = " << start_str << "  year = " << fScanYear
                                                                << "  fpday = " << fScanStartFpDay << eom);

    // Derive accumulation period from the time axis.
    auto* time_ax = &(std::get< TIME_AXIS >(*in));
    std::size_t nap = time_ax->GetSize();
    if(nap < 2)
    {
        msg_warn("calibration", "MHO_AdhocFlagging: only " << nap << " AP(s); cannot determine AP size." << eom);
        // Non-fatal: with one AP there is nothing meaningful to flag.
        return true;
    }
    fAccPeriod = time_ax->at(1) - time_ax->at(0);

    // Load flag tables for both stations.
    bool ref_ok = LoadFlagFile(0);
    bool rem_ok = LoadFlagFile(1);
    if(!ref_ok)
    {
        msg_error("calibration", "MHO_AdhocFlagging: failed to load reference station flag file." << eom);
    }
    if(!rem_ok)
    {
        msg_error("calibration", "MHO_AdhocFlagging: failed to load remote station flag file." << eom);
    }
    return ref_ok && rem_ok;
}

bool MHO_AdhocFlagging::InitializeOutOfPlace(const weight_type* in, weight_type* out)
{
    out->Copy(*in);
    return InitializeInPlace(out);
}

// ---------------------------------------------------------------------------
// ExecuteInPlace / OutOfPlace
// ---------------------------------------------------------------------------

bool MHO_AdhocFlagging::ExecuteInPlace(weight_type* in)
{
    // Both files empty -> nothing to do.
    if(fFlagFile[0].empty() && fFlagFile[1].empty())
    {
        return true;
    }

    auto* chan_ax = &(std::get< CHANNEL_AXIS >(*in));
    auto* time_ax = &(std::get< TIME_AXIS >(*in));
    auto* pp_ax = &(std::get< POLPROD_AXIS >(*in));

    std::size_t nch = chan_ax->GetSize();
    std::size_t nap = time_ax->GetSize();
    std::size_t npp = pp_ax->GetSize();

    std::size_t n_zeroed = 0;

    for(std::size_t ch = 0; ch < nch; ch++)
    {
        // Get the sideband for this channel.  Default to "U" if not labelled.
        std::string net_sideband;
        bool has_sideband = chan_ax->RetrieveIndexLabelKeyValue(ch, fSidebandKey, net_sideband);
        if(!has_sideband)
        {
            msg_warn("calibration", "MHO_AdhocFlagging: channel " << ch << " has no net_sideband label; skipping." << eom);
            continue;
        }

        // Channel index ch directly maps to byte index fr in the flag file,
        // matching the legacy convention where 'fr' is the frequency channel ordinal.
        // If ch >= MAX_FLAG_FREQS the byte index wraps (flag files rarely reach 64 channels).
        // eventually we need to eliminate this restriction (and allow more than 64 channels), but for now,
        // that is the legacy format we support
        std::size_t fr = ch % MAX_FLAG_FREQS;

        for(std::size_t t = 0; t < nap; t++)
        {
            // AP centre time in fractional days since beginning of year.
            double ap_center_sec = time_ax->at(t) + 0.5 * fAccPeriod;
            double ap_center_fpday = fScanStartFpDay + ap_center_sec / 86400.0;

            //std::cout<<std::setprecision(15)<<"ap_center_fpday: "<<ap_center_fpday<<std::endl;

            const uint8_t* ref_bytes = LookupFlagBytes(0, ap_center_fpday);
            const uint8_t* rem_bytes = LookupFlagBytes(1, ap_center_fpday);

            uint8_t combined = ref_bytes[fr] & rem_bytes[fr];

            bool retain;
            if(net_sideband == "U")
            {
                // USB is retained when any USB bit (0xAA) is set.
                retain = (combined & USB_MASK) != 0u;
            }
            else if(net_sideband == "L")
            {
                // LSB is retained when any LSB bit (0x55) is set.
                retain = (combined & LSB_MASK) != 0u;
            }
            else
            {
                // Unknown sideband label - do not flag.
                retain = true;
            }

            //std::cout<<"retained? = "<<retain<<std::endl;

            if(!retain)
            {
                // Zero out all pol-products for this (channel, AP).
                for(std::size_t pp = 0; pp < npp; pp++)
                {
                    in->SubView(pp, ch, t) *= 0.0;
                }
                n_zeroed++;
            }
        }
    }

    if(n_zeroed > 0)
    {
        msg_debug("calibration", "MHO_AdhocFlagging: zeroed " << n_zeroed << " (channel, AP) weight entries." << eom);

        // Recompute total_summed_weights and the effective AP count after zeroing flagged APs.
        // Both must happen *before* passband/notches rescale weights, so that integration
        // time and rate-error count only unflagged AP time slots (matching legacy fourfit).
        double new_total = 0.0;
        int effective_naps = 0;
        for(std::size_t t = 0; t < nap; t++)
        {
            double ap_sum = 0.0;
            for(std::size_t pp2 = 0; pp2 < npp; pp2++)
                for(std::size_t ch2 = 0; ch2 < nch; ch2++)
                {
                    auto freq_view = in->SubView(pp2, ch2, t);
                    for(auto it = freq_view.begin(); it != freq_view.end(); ++it)
                        ap_sum += *it;
                }
            new_total += ap_sum;
            if(ap_sum > 0.0)
                effective_naps++;
        }
        in->Insert("total_summed_weights", new_total);
        if(fParameterStore != nullptr)
        {
            fParameterStore->Set("/fringe/total_summed_weights", new_total);
            fParameterStore->Set("/config/total_naps", effective_naps);
        }
        msg_debug("calibration",
                  "MHO_AdhocFlagging: updated total_summed_weights = " << new_total
                      << "  effective_naps = " << effective_naps << eom);
    }

    return true;
}

bool MHO_AdhocFlagging::ExecuteOutOfPlace(const weight_type* in, weight_type* out)
{
    out->Copy(*in);
    return ExecuteInPlace(out);
}

} // namespace hops
