#include "MHO_StationStructWrapper.hh"
#include "MHO_DateStructWrapper.hh"
#include "MHO_ChanStructWrapper.hh"

// struct station_struct
//     {
//     short               start_offset;           /* Seconds */
//     short               stop_offset;            /* Seconds */
//     float               start_tapepos;          /* Meters */
//     short               tape_motion;            /* Defines above */
//     short               early_start;            /* Seconds */
//     short               late_finish;            /* Seconds */
//     short               tape_gap;               /* Seconds */
//     char                subpass;                /* Standard vex meaning */
//     short               passno;                 /* Standard vex meaning */
//     short               drive_no;
//     short               site_type;              /* Defines above */
//     char                site_name[9];
//     char                site_id[3];             /* International 2 char code */
//     char                mk4_site_id;            /* 1-char correlator alias */
//     double              coordinates[3];         /* Meters */
//     struct date         coordinate_epoch;       /* Standard Mk4 struct */
//     double              site_velocity[3];       /* Meters/sec */
//     float               zenith_atm;             /* Seconds */
//     char                occucode[5];            /* Standard 4-char code */
//     short               axis_type;              /* Defines above */
//     float               axis_offset;            /* Meters */
//     short               recorder_type;          /* Defines above */
//     short               rack_type;              /* Defines above */
//     float               record_density;         /* Bits/inch */
//     float               tape_length;            /* Meters */
//     short               recorder_id;            /* Unique integer */
//     float               clock_early;            /* Seconds */
//     struct date         clockrate_epoch;        /* Standard Mk4 struct */
//     float               clockrate;              /* sec/sec */
//     char                tape_id[9];             /* Standard tape label */
//     double              samplerate;             /* Samples/sec */
//     short               track_format;           /* Defines above */
//     short               modulation;             /* Defines above */
//     short               bits_sample;            /* 1 or 2 */
//     short               multiplex_ratio;        /* 1, 2 or 4 */
//     char                pass_direction;         /* F or R */
//     float               head_position[4];       /* Meters */
//     short               roll;                   /* Defines above */
//     short               roll_increment;         /* Frames */
//     float               roll_period;            /* Seconds */
//     short               roll_seq[5][34][32];    /* Track numbers */
//     struct chan_struct  channels[MAX_CHAN];
//     };


namespace hops
{


void
MHO_StationStructWrapper::DumpToJSON(json& json_obj)
{

    //     short               start_offset;           /* Seconds */
    //     short               stop_offset;            /* Seconds */
    //     float               start_tapepos;          /* Meters */
    //     short               tape_motion;            /* Defines above */
    //     short               early_start;            /* Seconds */
    //     short               late_finish;            /* Seconds */
    //     short               tape_gap;               /* Seconds */

    json_obj["start_offset"] = fStation.start_offset;
    json_obj["stop_offset"] = fStation.stop_offset;



    json_obj["drive_no"] = fStation.drive_no;
    json_obj["site_type"] = fStation.site_type;
    json_obj["site_name"] = std::string(fStation.site_name);
    json_obj["site_id"] = std::string(fStation.site_id);
    json_obj["mk4_site_id"] = std::string( &(fStation.mk4_site_id) ,1);
    json_obj["coordinates"] = std::vector<double>(fStation.coordinates, fStation.coordinates + 3);

    MHO_DateStructWrapper epoch(fStation.coordinate_epoch);
    epoch.DumpToJSON(json_obj["coordinate_epoch"]);
    json_obj["site_velocity"] = std::vector<double>(fStation.site_velocity, fStation.site_velocity + 3);
    json_obj["zenith_atm"] = fStation.zenith_atm;

    //TODO FIXME... do we need these?
    //json_obj["occucode"] = std::string(fStation.occucode);
    json_obj["axis_type"] = fStation.axis_type;
    json_obj["axis_offset"] = fStation.axis_offset;


    json_obj["clock_early"] = fStation.clock_early;
    MHO_DateStructWrapper clk_epoch(fStation.clockrate_epoch);
    clk_epoch.DumpToJSON(json_obj["clockrate_epoch"]);
    json_obj["clockrate"] = fStation.clockrate;

    json_obj["samplerate"] = fStation.samplerate;
    json_obj["bits_sample"] = fStation.bits_sample;

    std::vector< json > channels;
    for(int i=0; i<MAX_CHAN; i++)
    {
        //only write out channels which have a name which is defined
        if( std::string(this->fStation.channels[i].chan_name) != "")
        {
            json tmp;
            MHO_ChanStructWrapper aChan(fStation.channels[i]);
            aChan.DumpToJSON(tmp);
            channels.push_back(tmp);
        }
    }

    json_obj["channels"] = channels;

}


}
