#include "MHO_ScanStructWrapper.hh"
#include "MHO_DateStructWrapper.hh"
#include "MHO_SourceStructWrapper.hh"
#include "MHO_StationStructWrapper.hh"

#include <iostream>

namespace hops{




    // struct scan_struct
    //     {
    //     char                        filename[256];  /* Name of input vex file */
    //     short                       exper_num;      /* Standard 4-digit */
    //     char                        exper_name[32];
    //     char                        correlator[32];
    //     char                        scan_name[32];
    //     struct date                 start_time;     /* Standard Mk4 struct */
    //     struct date                 ffit_reftime;   /* Standard Mk4 struct */
    //     float                       tai_utc;        /* EOP parameters (global) */
    //     float                       a1_tai;
    //     int                         neop;           /* Number of eop entries */
    //     struct date                 eop_reftime;    /* Time of 1st entry */
    //     int                         eop_interval;   /* Seconds */
    //     float                       ut1_utc[10];    /* Seconds */
    //     float                       x_wobble[10];   /* Radians */
    //     float                       y_wobble[10];   /* Radians */
    //     struct source_struct        src;
    //     short                       nst;            /* Number of st elements */
    //     struct station_struct       *st;            /* Allocated */
    //     };


void
MHO_ScanStructWrapper::DumpToJSON(json& json_obj)
{

    json_obj["filename"] = std::string(fScanStruct.filename);
    json_obj["exper_num"] = fScanStruct.exper_num;
    json_obj["exper_name"] = std::string(fScanStruct.exper_name);
    json_obj["correlator"] = std::string(fScanStruct.correlator);
    json_obj["scan_name"] = std::string(fScanStruct.scan_name);

    MHO_DateStructWrapper start_time(fScanStruct.start_time);
    start_time.DumpToJSON(json_obj["start_time"]);

    MHO_DateStructWrapper ffit_reftime(fScanStruct.ffit_reftime);
    ffit_reftime.DumpToJSON(json_obj["ffit_reftime"]);

    if(!is_float_unset(fScanStruct.tai_utc)){json_obj["tai_utc"] = fScanStruct.tai_utc;}

    if(!is_float_unset(fScanStruct.a1_tai)){json_obj["a1_tai"] = fScanStruct.a1_tai;}

    if(!is_short_unset(fScanStruct.neop)){json_obj["neop"] = fScanStruct.neop;}

    MHO_DateStructWrapper eop_reftime(fScanStruct.eop_reftime);
    eop_reftime.DumpToJSON(json_obj["eop_reftime"]);

    if(!is_int_unset(fScanStruct.eop_interval)){json_obj["eop_interval"] = fScanStruct.eop_interval;}

    std::vector<float> ut1_utc;
    for(int i=0; i<10; i++)
    {
        if(!is_float_unset(fScanStruct.ut1_utc[i])){ut1_utc.push_back(fScanStruct.ut1_utc[i]);}
    }

    if(ut1_utc.size() != 0)
    {
        json_obj["ut1_utc"] = ut1_utc;
    }

    std::vector<float> x_wobble;
    for(int i=0; i<10; i++)
    {
        if(!is_float_unset(fScanStruct.x_wobble[i])){x_wobble.push_back(fScanStruct.x_wobble[i]);}
    }
    if(ut1_utc.size() != 0)
    {
        json_obj["x_wobble"] = x_wobble;
    }


    std::vector<float> y_wobble;
    for(int i=0; i<10; i++)
    {
        if(!is_float_unset(fScanStruct.y_wobble[i])){y_wobble.push_back(fScanStruct.y_wobble[i]);}
    }
    if(ut1_utc.size() != 0)
    {
        json_obj["y_wobble"] = y_wobble;
    }

    MHO_SourceStructWrapper src(fScanStruct.src);
    src.DumpToJSON(json_obj["src"]);

    json_obj["nst"] = fScanStruct.nst;

    std::vector<json> stations;
    //skipping station_struct
    // for( int i=0; i< fScanStruct.nst; i++)
    for( int i=0; i<2; i++)
    {
        json tmp;
        MHO_StationStructWrapper station(fScanStruct.st[i]);
        station.DumpToJSON(tmp);
        stations.push_back(tmp);
    }
    json_obj["stations"] = stations;

}



}
