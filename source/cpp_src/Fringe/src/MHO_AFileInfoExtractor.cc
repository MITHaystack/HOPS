
#include "MHO_FileKey.hh"
#include "MHO_BinaryFileInterface.hh"
#include "MHO_ContainerDictionary.hh"
#include "MHO_ContainerDefinitions.hh"
#include "MHO_ContainerFileInterface.hh"
#include "MHO_ContainerStore.hh"

#include "MHO_AFileDefinitions.hh"
#include "MHO_AFileInfoExtractor.hh"


//
// #define CURRENT_VERSION 5
// #define AFILEMX_VERSION 6
//

//
//

//

//
// typedef struct {
//         short                   version;        /* Disk format version number */
//         short                   expt_no;        /* Experiment serial # */
//         int                     time_tag;       /* Seconds since 0h, Jan 1 1980 */
//         char                    source[32];     /* Source name */
//         char                    freq_code;      /* type 2 only */
//         char                    mode;           /* recording mode */
//         char                    triangle[4];    /* Stations in closure triangle */
//         char                    root_id[3][7];  /* 3 comma-separated root id codes */
//         short                   extent_no[3];   /* From HP-1000 system */
//         short                   length[3];      /* scan lengths in seconds */
//         char                    scan_quality;   /* closure quality code, scan-derived */
//         char                    data_quality;   /* closure quality code, calc'd by average */
//         int                     esdesp;         /* Max diff around triangle per digit */
//         float                   bis_amp;        /* Bispectrum amplitude, e-12 */
//         float                   bis_snr;        /* Bispectrum SNR */
//         float                   bis_phas;       /* Bispectrum phase = closure phase */
//         float                   csbdelay;       /* closure singleband delay usec */
//         float                   cmbdelay;       /* closure multiband delay usec */
//         float                   ambiguity;      /* mbdelay ambiguity */
//         float                   cdelay_rate;    /* closure delay rate psec/sec */
//         float                   elevation[3];   /* By station */
//         float                   azimuth[3];     /* By station */
//         short                   epoch[2];       /* reference epoch mins,secs */
//         double                  ref_freq;       /* Reference frequency */
//
//                                         /* Added for version 3 */
//
//         short                   duration;       /* Nominal duration of scan (secs) */
//         short                   offset;         /* mean time minus scan_time (sec) */
//         char                    datatype[3];    /* Meaning TBD */
//
//                                         /* Added for version 4 */
//
//         short                   scan_offset;    /* time_tag minus scan time */
//         int                     lags;           /* Number of lags in correlation */
//         short                   cotime;         /* Coherence time of worst b'line */
//
//                                         /* Added for version 5 */
//
//         char                    scan_id[32];    /* From VEX, not necessarily scantime */
// } trianglesum;
//
// /* The total length of this structure in bytes is 208 */
//
//
//
// typedef struct {
//         short                   version;        /* Disk format version number */
//         short                   expt_no;        /* Experiment serial # */
//         int                     time_tag;       /* Seconds since 0h, Jan 1 1980 */
//         char                    source[32];     /* Source name */
//         char                    freq_code;      /* type 2 only */
//         char                    mode;           /* recording mode */
//         char                    quad[5];        /* Stations in closure quad */
//         char                    root_id[42];    /* 6 comma-separated root id codes */
//         short                   extent_no[6];   /* From HP-1000 system */
//         short                   length[6];      /* scan lengths in seconds */
//         char                    quality;        /* closure quality code */
//         int                     esdesp;         /* Max diff around quad per digit */
//         float                   cl_amp;         /* Closure amplitude */
//         float                   elevation[4];   /* By station */
//         float                   azimuth[4];     /* By station */
//         short                   epoch[2];       /* reference epoch mins,secs */
//         double                  ref_freq;       /* Reference frequency */
//
//                                         /* Added for version 3 */
//
//         short                   duration;       /* Nominal duration of scan (secs) */
//         short                   offset;         /* mean time minus scan_time (sec) */
//         char                    datatype[3];    /* Meaning TBD */
//
//                                         /* Added for version 4 */
//
//         short                   scan_offset;    /* time_tag minus scan time */
//         int                     lags;           /* Number of lags in correlation */
//
//                                         /* Added for version 5 */
//
//         char                    scan_id[32];    /* From VEX, not necessarily scantime */
// } quadsum;
//
//
//
//






namespace hops
{

// mho_json MHO_AFileInfoExtractor::summarize_root_file(std::string filename)
// {
// }
    // typedef struct {
    //         short                   version;        /* Disk format version number */
    //         char                    fname[6];       /* FMGR name without "<" */
    //         short                   expt_no;        /* Experiment serial # */
    //         short                   extent_no;      /* From HP-1000 system */
    //         short                   size;           /* file size in 256-byte blocks */
    //         char                    corel_vers;     /* Corel version used */
    //         int                     procdate;       /* Creation date for this extent */
    //         int                     time_tag;       /* Secs since 0h, Jan 1 1980 */
    //         short                   ssec;           /* scan time seconds, if available */
    //         char                    source[32];     /* Source name */
    //         char                    stations[20];   /* station list */
    //         char                    root_id[7];     /* Unique 6-char root id code */
    //         short                   archiv;         /* A-file number */
    //
    //                                         /* Added for version 5 */
    //
    //         char                    scan_id[32];    /* From VEX, not necessarily scantime */
    // } rootsum;
    //
    // /* The total length of this structure in bytes is 120 */


// mho_json
// MHO_AFileInfoExtractor::summarize_corel_file(std::string filename)
// {
// }
    // typedef struct {
    //         short                   version;        /* Disk format version number */
    //         char                    fname[6];       /* FMGR name without "<" */
    //         short                   expt_no;        /* Experiment serial # */
    //         short                   extent_no;      /* From HP-1000 system */
    //         short                   size;           /* file size in 256-byte blocks */
    //         char                    corel_vers;     /* Corel version used */
    //         int                     procdate;       /* Creation date for this extent */
    //         int                     time_tag;       /* Secs since 0h, Jan 1 1980 */
    //         short                   ssec;           /* scan time seconds, if available */
    //         char                    source[32];     /* Source name */
    //         char                    baseline[3];    /* standard baseline id */
    //         char                    quality;        /* corel quality code */
    //         short                   startsec;       /* UT seconds of scheduled start */
    //         short                   sduration;      /* Scheduled duration of scan secs */
    //         short                   corstart;       /* Correlation start in secs after sch. strt */
    //         short                   corstop;        /* Correlation stop in secs after sch. strt */
    //         short                   refdrive;       /* Correlator tape drive # reference station */
    //         short                   remdrive;       /* Correlator tape drive # reference station */
    //         short                   eqts;           /* # of EQTs configured into this b'line (?) */
    //         char                    freqs[3];       /* Frequencies processed */
    //         float                   refclock_err;   /* Ref station apriori clock error (usec) */
    //         float                   clock_diff;     /* Difference between station clocks (usec) */
    //         char                    root_id[7];     /* Unique 6-char root id code */
    //         int                     status;         /* Correlation status bits */
    //         short                   archiv;         /* A-file number */
    //
    //                                         /* Added for version 4 */
    //
    //         int                     lags;           /* Number of lags in correlation */
    //
    //                                         /* Added for version 5 */
    //
    //         char                    scan_id[32];    /* From VEX, not necessarily scantime */
    // } corelsum;
    //
    // /* The total length of this structure in bytes is 140 */

// mho_json
// MHO_AFileInfoExtractor::summarize_station_file(std::string filename)
// {
//
// }

mho_json
MHO_AFileInfoExtractor::SummarizeFringeFile(std::string filename)
{
    mho_json fsum;
    if(filename.find("frng") == std::string::npos)
    {
        //not a fringe file, skip this
        msg_error("fringe", "the file: "<< filename<<" is not a HOPS4 fringe (.frng) file"<<eom);
        return fsum;
    }

    //split the filename (not strictly necessary, but used to extract the extent number)
    //for example: 23 in 'GE.X.XX.345F47.23.frng'
    MHO_Tokenizer tokenizer;
    tokenizer.SetDelimiter(".");
    tokenizer.SetIncludeEmptyTokensFalse();
    tokenizer.SetString(&filename);
    std::vector< std::string > tokens;
    tokenizer.GetTokens(&tokens);
    if(tokens.size() != 6)
    {
        //not a fringe file, skip this
        msg_error("fringe", "could not parse the file name: "<< filename<<eom);
        return fsum;
    }
    //grab the extent number
    std::stringstream ss;
    ss << tokens[4];
    int extent_no;
    ss >> extent_no;

    //pull the fringe file format definition
    std::string file_type = "frng";
    MHO_AFileDefinitions adef;
    mho_json aformat = adef.GetAFileFormat(file_type);
    mho_json fringe_format = aformat["fake_summary"];
    int version = fringe_format["default_version"];
    mho_json fields = fringe_format["fields_v1"];

    //to pull out fringe data, we are primarily interested in the 'MHO_ObjectTags' object
    //get uuid for MHO_ObjectTags object
    MHO_ContainerDictionary cdict;
    MHO_UUID tag_uuid = cdict.GetUUIDFor<MHO_ObjectTags>();

    //pull all the keys and byte offsets for each object
    std::vector< MHO_FileKey > ikeys;
    std::vector< std::size_t > byte_offsets;
    MHO_BinaryFileInterface inter;
    inter.ExtractFileObjectKeysAndOffsets(filename, ikeys, byte_offsets);

    //loop over keys and offsets, looking for tags offset
    bool found = false;
    std::size_t offset_bytes = 0;
    for(std::size_t i=0; i<ikeys.size(); i++)
    {
        if(ikeys[i].fTypeId == tag_uuid)
        {
            offset_bytes = byte_offsets[i];
            found = true;
            break; //only first tag object is used
        }
    }

    if(found)
    {
        inter.OpenToReadAtOffset(filename, offset_bytes);
        MHO_ObjectTags obj;
        MHO_FileKey obj_key;
        //we read the tags object
        //now pull the pol-products and frequency groups info
        //and check them agains the command line arguments
        bool ok = inter.Read(obj, obj_key);
        std::vector< std::string > tags;

        if(ok)
        {
            //pull the parameters
            MHO_ParameterStore paramStore;
            mho_json param_data;
            bool param_ok = obj.GetTagValue("parameters", param_data);
            if(!param_ok)
            {
                msg_error("fringe", "could not read parameters object from: "<< filename << eom);
                return fsum;
            }
            paramStore.FillData(param_data);

            //pull the plot data
            MHO_ParameterStore plotData;
            mho_json plot_data;
            bool plot_ok = obj.GetTagValue("plot_data", plot_data);
            if(!plot_ok)
            {
                msg_error("fringe", "could not read plot_data object from: "<< filename << eom);
                return fsum;
            }
            plotData.FillData(plot_data);

            //loop over all the fields defined by the format and extract the info we need
            for(auto it = fields.begin(); it != fields.end(); it++)
            {
                std::string field_name = it->get<std::string>();
                if(aformat.contains(field_name))
                {
                    if(aformat[field_name].contains("source_object"))
                    {
                        std::string source_object = aformat[field_name]["source_object"].get<std::string>();
                        if(aformat[field_name].contains("path") && aformat[field_name].contains("type") )
                        {
                            std::string path = aformat[field_name]["path"].get<std::string>();
                            std::string type = aformat[field_name]["type"].get<std::string>();
                            if(source_object == "parameters"){RetrieveParameter(fsum, field_name, paramStore, path, type);}
                            if(source_object == "plot_data"){RetrieveParameter(fsum, field_name, plotData, path, type);}
                            if(source_object == "local_variable")
                            {
                                //only a limited number of these, treat each one individually
                                if(field_name == "fname"){fsum[field_name] = filename;}
                                if(field_name == "version"){fsum[field_name] = version;}
                                if(field_name == "extent_no"){fsum[field_name] = extent_no;}
                            }
                            //deprecated or unavailable objects get a default value
                            if(source_object == "none" && aformat[field_name].contains("default") ){fsum[field_name] = aformat[field_name]["default"];}
                        }
                    }
                }
            }
        }
        else
        {
            msg_error("fringe", "could not read MHO_ObjectTags from: "<< filename << eom);
        }
        inter.Close();
    }
    else
    {
        msg_error("fringe", "no MHO_ObjectTags object found in file: "<< filename << eom);
    }

    return fsum;
}


std::string
MHO_AFileInfoExtractor::ConvertToAlistRow(const mho_json& fringe_data, const std::string& delim)
{
    std::stringstream row_data;

    //pull the fringe file format definition
    std::string file_type = "frng";
    MHO_AFileDefinitions adef;
    mho_json aformat = adef.GetAFileFormat(file_type);
    mho_json fringe_format = aformat["fake_summary"];
    int version = fringe_format["default_version"];
    mho_json fields = fringe_format["fields_v1"];

    //loop over all the fields defined by the format and extract the info we need
    for(auto it = fields.begin(); it != fields.end(); it++)
    {
        std::string field_name = it->get<std::string>();
        if(aformat.contains(field_name) && fringe_data.contains(field_name) )
        {
            if(aformat[field_name].contains("pformat") && aformat[field_name].contains("type") )
            {
                std::string type = aformat[field_name]["type"].get<std::string>();
                std::string pformat = aformat[field_name]["pformat"].get<std::string>();
                std::string print_value = RetrieveParameterAsString(fringe_data, field_name, type, pformat );
                row_data << print_value;
                //if not the last element push out a delimiter
                auto it2 = it;
                if(it2++ != fields.end()){row_data << delim;}
            }
        }
    }
    return row_data.str();
}

void
MHO_AFileInfoExtractor::RetrieveParameter(mho_json& obj, const std::string& name,
                                          const MHO_ParameterStore& paramStore,
                                          const std::string& path,
                                          const std::string& type)
{
    //TODO...eventually replace missing values with default values specified from the format
    par_type ptype = DetermineParameterType(type);
    switch( ptype )
    {
        case int_type:
            if( paramStore.IsPresent(path)){ obj[name] = paramStore.GetAs<int>(path); }
            else{ obj[name] = 0;}
        break;
        case int64_type:
            if( paramStore.IsPresent(path)){ obj[name] = paramStore.GetAs<int64_t>(path); }
            else{ obj[name] = 0;}
        break;
        case double_type:
            if( paramStore.IsPresent(path)){ obj[name] = paramStore.GetAs<double>(path); }
            else{ obj[name] = 0;}
        break;
        case string_type:
            if( paramStore.IsPresent(path)){ obj[name] = paramStore.GetAs<std::string>(path); }
            else{ obj[name] = "";}
        break;
        case bool_type:
            if( paramStore.IsPresent(path)){ obj[name] = paramStore.GetAs<bool>(path); }
            else{ obj[name] = 0;}
        break;
    };
}


par_type
MHO_AFileInfoExtractor::DetermineParameterType(std::string etype)
{
    if(etype == "int"){return int_type;}
    if(etype == "int64_t"){return int64_type;}
    if(etype == "double"){return double_type;}
    if(etype == "string"){return string_type;}
    if(etype == "bool"){return bool_type;}
    return unknown_type;
}


std::string MHO_AFileInfoExtractor::RetrieveParameterAsString(const mho_json& obj,
                                                              const std::string& name,
                                                              const std::string& type,
                                                              const std::string& pformat)
{
    par_type ptype = DetermineParameterType(type);
    switch( ptype )
    {
        case int_type:
        {
                int value = 0;
                if( obj.contains(name) ){value = obj[name].get<int>(); }
                return ConvertToString(value, pformat);
        }
        break;
        case int64_type:
        {
            int64_t value = 0;
            if( obj.contains(name)){value = obj[name].get<int64_t>(); }
            return ConvertToString(value, pformat);
        }
        break;
        case double_type:
        {
            double value = 0;
            if( obj.contains(name)){value = obj[name].get<double>(); }
            return ConvertToString(value, pformat);
        }
        break;
        case string_type:
        {
            std::string value;
            if( obj.contains(name)){value = obj[name].get<std::string>(); }
            return ConvertToString(value, pformat);
        }
        break;
        case bool_type:
        {
            bool value;
            if( obj.contains(name)){value = obj[name].get<bool>();}
            return ConvertToString(value, pformat);
        }
        break;
    };

    //if we made it here just return empty string
    return std::string("");
}



    //
    // typedef struct {
    //         short                   version;        /* Disk format version number */
    //         char                    fname[6];       /* FMGR name without "<" */
    //         short                   expt_no;        /* Experiment serial # */
    //         short                   extent_no;      /* From HP-1000 system */
    //         short                   length;         /* scan length in seconds */
    //         char                    corel_vers;     /* Corel version used */
    //         int                     procdate;       /* Creation date for this extent */
    //         int                     time_tag;       /* Seconds since 0h, Jan 1 1980 */
    //         short                   ssec;           /* scan time seconds, version 1 */
    //         char                    source[32];     /* Source name */
    //         char                    baseline[3];    /* standard baseline id */
    //         char                    quality;        /* frnge quality code */
    //         char                    freq_code;      /* type 2 only */
    //         char                    mode;           /* recording mode */
    //         short                   no_freq;        /* number of freqs through frnge */
    //         short                   archiv;         /* A-file number */
    //         char                    reftape[9];     /* Reference tape label */
    //         char                    remtape[9];     /* Remote tape label */
    //         float                   amp;            /* Correlation amplitude */
    //         float                   snr;            /* SNR from frnge, v1 clips at 9999 */
    //         float                   resid_phas;     /* residual earth-centered phase deg */
    //         float                   sbdelay;        /* resid singleband delay usec */
    //         float                   mbdelay;        /* resid multiband delay usec */
    //         float                   delay_rate;     /* resid delay rate psec/sec */
    //         int                     esdesp;         /* Various numbers describing data */
    //         short                   epoch[2];       /* reference epoch mins,secs */
    //         float                   total_phas;     /* tot earth-centered phase deg */
    //         double                  total_rate;     /* tot delay rate usec/sec */
    //         double                  total_mbdelay;  /* tot multiband delay usec */
    //         float                   total_sbresid;  /* tot sbdelay - mbdelay usec */
    //         float                   ambiguity;      /* mbdelay ambiguity */
    //         short                   pcals[4];       /* Phasecals deg,  ref1,reflast, */
    //         char                    root_id[7];     /* Unique 6-char root id code */
    //
    //                                         /* Added for version 2 */
    //
    //         double                  ref_freq;       /* Reference frequency */
    //         char                    datatype[3];    /* Origin and phase type */
    //         float                   ref_elev;       /* Reference elevation */
    //         float                   rem_elev;       /* Remote elevation */
    //         float                   ref_az;         /* Reference azimuth */
    //         float                   rem_az;         /* Remote azimuth */
    //         float                   u;              /* u in megalambda */
    //         float                   v;              /* v in megalambda */
    //         short                   parents[4];     /* Parent corel extent(s) */
    //
    //                                         /* Added for version 3 */
    //
    //         short                   duration;       /* Nominal duration of scan (secs) */
    //         short                   offset;         /* mean time minus scan_time (sec) */
    //
    //                                         /* Added for version 4 */
    //
    //         short                   scan_offset;    /* time_tag minus scan time */
    //         int                     lags;           /* Number of lags in correlation*/
    //         float                   phase_snr;      /* When independent of amp. snr */
    //         short                   srch_cotime;    /* Coh. time for max. snr (sec) */
    //         short                   noloss_cotime;  /* Coh. time for negligible loss (sec) */
    //
    //                                         /* Added for version 5 */
    //
    //         char                    scan_id[32];    /* From VEX, not necessarily scantime */
    //         char                    polarization[3]; /* RR, LL, RL or LR */
    //         char                    errcode;        /* for Mk3-style letter codes */
    //
    //                                         /* Added for version 6 */
    //
    //         float                   ra_hrs;         /* derived from sky_coord */
    //         float                   dec_deg;        /* derived from sky_coord */
    //
    //         float                   resid_delay;    /* N*AMB + MBD to match SBD */
    //
    // } fringesum;
    //
    // /* The total length of this structure in bytes is 280 */
    //


}//end namespace
