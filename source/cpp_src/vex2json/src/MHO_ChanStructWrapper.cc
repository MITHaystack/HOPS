#include "MHO_ChanStructWrapper.hh"

// struct chan_struct
//     {
//     char                chan_name[32];          /* External channel name */
//     char                polarization;           /* R or L */
//     double              sky_frequency;          /* Hz */
//     char                net_sideband;           /* U or L */
//     double              bandwidth;              /* Hz */
//     char                band_id[32];            /* Linkword (internal use) */
//     char                chan_id[32];            /* Linkword (internal use) */
//     char                bbc_id[32];             /* Linkword (internal use) */
//     char                pcal_id[32];            /* Linkword (internal use) */
//     char                if_id[32];              /* Linkword (internal use) */
//     short               bbc_no;                 /* Physical BBC# */
//     char                if_name[8];             /* Physical IF name */
//     double              if_total_lo;            /* Hz */
//     char                if_sideband;            /* U or L */
//     float               pcal_spacing;           /* Hz */
//     float               pcal_base_freq;         /* Hz */
//     short               pcal_detect[16];        /* Integer tone #s */
//     short               sign_tracks[4];         /* Track #s */
//     short               sign_headstack;         /* 1-4 */
//     short               mag_tracks[4];          /* Track #s */
//     short               mag_headstack;          /* 1-4 */
//     };

namespace hops
{

void
MHO_ChanStructWrapper::DumpToJSON(json& json_obj)
{
    json_obj["chan_name"] = std::string(fChan.chan_name);
    json_obj["polarization"] = std::string( &(fChan.polarization), 1);
    json_obj["sky_frequency"] = fChan.sky_frequency;
    json_obj["net_sideband"] = std::string( &(fChan.net_sideband), 1);
    json_obj["bandwidth"] = fChan.bandwidth;

    //TODO FIXME
    //not sure if the linkword elements are needed, likely they can be removed?
    // json_obj["band_id"] = fChan.band_id;
    // json_obj["chan_id"] = fChan.chan_id;
    // json_obj["bbc_id"] = fChan.bbc_id;
    // json_obj["pcal_id"] = fChan.pcal_id;
    // json_obj["if_id"] = fChan.if_id;

    //TODO FIXME
    //not sure if these elements are needed...
    json_obj["bbc_no"] = fChan.bbc_no;
    json_obj["if_name"] = std::string(fChan.if_name);
    json_obj["if_total_lo"] = fChan.if_total_lo;
    json_obj["if_sideband"] = std::string( &(fChan.if_sideband), 1);
    json_obj["pcal_spacing"] = fChan.pcal_spacing;
    json_obj["pcal_base_freq"] = fChan.pcal_base_freq;

    //we have no use for the following element:
    //   short               pcal_detect[16];        /* Integer tone #s */

    //We have no use for the following elements (magnetic tape info)
    //     short               sign_tracks[4];         /* Track #s */
    //     short               sign_headstack;         /* 1-4 */
    //     short               mag_tracks[4];          /* Track #s */
    //     short               mag_headstack;          /* 1-4 */
}



}
