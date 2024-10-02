
#include "MHO_BinaryFileInterface.hh"
#include "MHO_ContainerDefinitions.hh"
#include "MHO_ContainerDictionary.hh"
#include "MHO_ContainerFileInterface.hh"
#include "MHO_ContainerStore.hh"
#include "MHO_FileKey.hh"

#include "MHO_AFileDefinitions.hh"
#include "MHO_AFileInfoExtractor.hh"

#define CHAR_BUF_SIZE 2048

/* prototyping a version 6 */

namespace hops
{

const char afile_com_char = '*';

/************************************************************************/

/************************************************************************/
/************************    Version 5    *******************************/
/************************************************************************/

//we only support alist v5 and v6
const char* fformat_v5 = "%1d %s 2 %2d %3d %3d %3d %4d %s %02d%03d-%02d%02d%02d %4d\
 %03d-%02d%02d%02d %3d %-8s %s %c%c %c%02d %2s %4d %6.2f %#5.4g %5.1f %#5.4g %2s %6.3f %8.5f\
 %6.4f %8.3f %4.1f %4.1f %5.1f %5.1f %7.4g %7.4g %06d %02d%02d %8.2f %5.1f %11.8f\
 %13.6f %5.3f %3d %3d\n";
/************************************************************************/

//root sum
char header50[] = "* ROOT   T X# SIZ         EXP# *************SCANID*************\
 PROCDATE     YEAR RUN#       SOURCE   STATIONS\n";
/************************************************************************/

//station sum
char header51[] = "* ROOT   T SIZ         EXP# *************SCANID*************\
 PROCDATE     YEAR RUN#       SOURCE   BS Q\
 DUR LAG DRVS FQ CLERR   CLDIFF  STATUS\n";
/************************************************************************/

//fringe sum
char header52[] = "%c ROOT   T F# DUR  LEN  OFF  EXP# *************SCANID*************\
 PROCDATE     YEAR TIME*TAG OFF   SOURCE   BS Q\
 FM#C PL LAGS   AMP    SNR  PH   SNR   TYP  SBDLY  MBDLY    AMB   \
 DRATE  ELEVATION AZIMUTH      U       V    ESDESP\
 EPCH REF_FREQ TPHAS  TOTDRATE   TOTMBDELAY  TOTSBDMMBD COHTIMES\n\
%c            (    sec    )                                          \
                  (deg)     (usec) (usec)  (usec)  (ps/s)   (deg)    \
   (deg)    (megalambda)          (mmss) (MHz)   (deg)  (usec/sec) \
  (usec)      (usec) (sec) *** NOT ALIGNED ***\n";
/************************************************************************/

//triangle sum
char header53[] = "* EXP# T *************SCANID*************\
 YEAR TIME*TAG OFF   SOURCE   FM LAGS TRI ROOTCODES\
 EXTENTS LENGTHS        DUR OFF Q1Q2 ESDESP AMP         SNR  CPHS     CSBD\
   CMBD     AMBIG     CRATE   ELEVATIONS  AZIMUTHS      EPOCH  REFFREQ\n";

/************************************************************************/

/************************************************************************/
/************************    Version 6    *******************************/
/************************************************************************/

const char* fformat_v6 = "%1d %s 2 %2d %3d %3d %3d %4d %8s %04d%03d-%02d%02d%02d\
 %4d %03d-%02d%02d%02d %3d %32s %2s %c%c\
 %c%02d %2s %5d\
 %#13.8g %#13.8g %11.6f %#11.6g %2s\
 %+12.9f %+12.9f %11.9f\
 %+11.6f %5.2f %5.2f %6.2f %6.2f %7.4g %7.4g %06d\
 %02d%02d %9.3f %10.6f %11.8f\
 %13.6f %+9.6f %8d %8d %+10.6f %+10.6f %+13.10f\n";

/************************************************************************/
char header60[] = "* ROOT   T X# SIZ         EXP# *************SCANID*************\
 PROCDATE     YEAR RUN#       SOURCE   STATIONS\n";

/************************************************************************/

char header61[] = "* ROOT   T SIZ         EXP# *************SCANID*************\
 PROCDATE     YEAR RUN#       SOURCE   BS Q\
 DUR LAG DRVS FQ CLERR   CLDIFF  STATUS\n";

/************************************************************************/
/* aligned to write_fsumm.c:fformat_v6 */
char header62[] = "%c col2 cl3 c4 cl5 cl6 cl7 col8     col9 \
         col10 cl11      col12 \
c13                            col14 15 16 \
c17 18 col19 \
        col20         col21       col22       col23 24 \
       col25        col26       col27 \
      col28 col29 \
col30  col31  col32   col33   col34  col35 \
cl36      col37      col38       col39         \
col40    col41    col42    col43      col44      col45         col46\n\
%c ROOT   T F# DUR LEN OFF EXP# SCANNAME PROCESSINGDATE YEAR DOY-HHMMSS \
OFF                       SOURCENAME BS QE\
 F#C PL #LAGS \
          AMP           SNR       PHASE        PSNR TY   \
     SBDLY        MBDLY         AMB     \
  DRATE ELref ELrem  AZref  AZrem       U       V ESDESP\
 EPCH   REF_FREQ   TOTPHASE    TOTDRATE   \
 TOTMBDELAY  TSBD-MBD SRCH-COH LOSS-COH         RA       DECL RESIDUALDELAY\n\
%c base64 2  . (s) (s) (s)    .        .              .    .          . \
  .                                .  . ..\
   .  .     . \
      (x10^4)             .       (deg)           . ..   \
    (usec)       (usec)      (usec)     \
 (ps/s) (deg) (deg)  (deg)  (deg)  (mega  lambda)      .\
 (ms)      (MHz)      (deg)  (usec/sec)   \
     (usec)    (usec)    (sec)    (sec)       (hr)      (deg)        (usec)\n";

/************************************************************************/

char header63[] = "\
%c col2 3     col4 col5       col6 cl7                             col8\
 c9 cl10 c11                col12\
       col13          col14\
 c16 c17 18 19  col20\
      col21    col22    col23 24   col25\
    col26     col27     col28\
          col29       col30 col31      col32   col33\n\
%c EXP# T SCANNAME YEAR PROCESSING OFF                           SOURCE\
 FM LAGS TRI RTCODE,RTCODE,RTCODE\
     EXTENTS        LENGTHS\
 DUR OFF Q1 Q2 ESDESP\
        AMP      SNR     CPHS DT    CSBD\
     CMBD     AMBIG     CRATE\
     ELEVATIONS    AZIMUTHS EPOCH    REFFREQ  COTIME\n\
%c    . .        .    .          .   .                                .\
  .    .   .                    .\
         (i)         (secs)\
 (s) (s)  .  .      .\
          .        .        .  .       .\
        .         .         .\
              .           .     .          .     (s)\n";

/************************************************************************/

void reset_buffer(char* buf)
{
    for(std::size_t i = 0; i < CHAR_BUF_SIZE; i++)
    {
        buf[i] = '\0';
    }
}

/*
 * Writes the 3-line header of the requested version and type
 */
std::string afile_header(int version, int type, char afile_com_char)
{
    std::string header_lines;
    std::stringstream ss;
    char buf[CHAR_BUF_SIZE];
    reset_buffer(buf);

    time_t now;
    /* Start with a time-stamp */
    now = time(NULL);
    sprintf(buf, "%c This file processed by %s, %s", afile_com_char, "alist4", ctime(&now));
    ss << buf;
    reset_buffer(buf);

    if(type != 2)
    {
        msg_error("fringe", "currently alist4 only supports fringe summaries" << eom);
        return std::string("* Invalid afile_header() call, so no header generated\n");
    }

    /* switch to appropriate header */
    int ret;
    switch(version)
    {
        case 5:
            ret = sprintf(buf, header52, '*', '*');
            break;
        case 6:
            ret = sprintf(buf, header62, afile_com_char, afile_com_char, afile_com_char);
            break;
        default:
            ret = sprintf(buf, "* Invalid afile_header() call, so no header generated\n");
            break;
    }

    ss << buf;
    return ss.str();
}

std::string MHO_AFileInfoExtractor::GetAlistHeader(int version, int type, char comment_char)
{
    return afile_header(version, type, comment_char);
}

bool MHO_AFileInfoExtractor::SummarizeFringeFile(std::string filename, mho_json& fsum)
{
    if(filename.find("frng") == std::string::npos)
    {
        //not a fringe file, skip this
        msg_error("fringe", "the file: " << filename << " is not a HOPS4 fringe (.frng) file" << eom);
        return false;
    }

    //split the filename (not strictly necessary, but used to extract the extent number)
    //for example: 23 in 'GE.X.XX.345F47.23.frng'
    MHO_Tokenizer tokenizer;
    tokenizer.SetDelimiter(".");
    tokenizer.SetIncludeEmptyTokensFalse();
    tokenizer.SetString(&filename);
    std::vector< std::string > tokens;
    tokenizer.GetTokens(&tokens);
    if(tokens.size() < 6)
    {
        //not a fringe file, skip this
        msg_error("fringe", "could not parse the file name: " << filename << eom);
        return false;
    }
    //grab the extent number
    std::stringstream ss;
    ss << tokens[tokens.size() - 2];
    int extent_no;
    ss >> extent_no;

    //pull the fringe file format definition
    std::string file_type = "frng";
    MHO_AFileDefinitions adef;
    mho_json aformat = adef.GetAFileFormat(file_type);
    mho_json fringe_format = aformat["fringe_summary"];
    //default is to extract everything in v6 (alist formatting is handled elsewhere)
    int version = 6;
    mho_json fields = fringe_format["fields_v6"];

    //to pull out fringe data, we are primarily interested in the 'MHO_ObjectTags' object
    //get uuid for MHO_ObjectTags object
    MHO_ContainerDictionary cdict;
    MHO_UUID tag_uuid = cdict.GetUUIDFor< MHO_ObjectTags >();

    //pull all the keys and byte offsets for each object
    std::vector< MHO_FileKey > ikeys;
    std::vector< std::size_t > byte_offsets;
    MHO_BinaryFileInterface inter;
    inter.ExtractFileObjectKeysAndOffsets(filename, ikeys, byte_offsets);

    //loop over keys and offsets, looking for tags offset
    bool found = false;
    std::size_t offset_bytes = 0;
    for(std::size_t i = 0; i < ikeys.size(); i++)
    {
        if(ikeys[i].fTypeId == tag_uuid)
        {
            offset_bytes = byte_offsets[i];
            found = true;
            break; //only first tag object is used
        }
    }

    bool ok = false;
    if(found)
    {
        inter.OpenToReadAtOffset(filename, offset_bytes);
        MHO_ObjectTags obj;
        MHO_FileKey obj_key;
        //we read the tags object
        //now pull the pol-products and frequency groups info
        //and check them agains the command line arguments
        ok = inter.Read(obj, obj_key);
        std::vector< std::string > tags;

        if(ok)
        {
            //pull the parameters
            MHO_ParameterStore paramStore;
            mho_json param_data;
            bool param_ok = obj.GetTagValue("parameters", param_data);
            if(!param_ok)
            {
                msg_error("fringe", "could not read parameters object from: " << filename << eom);
                return false;
            }
            paramStore.FillData(param_data);

            //pull the plot data
            MHO_ParameterStore plotData;
            mho_json plot_data;
            bool plot_ok = obj.GetTagValue("plot_data", plot_data);
            if(!plot_ok)
            {
                msg_error("fringe", "could not read plot_data object from: " << filename << eom);
                return false;
            }
            plotData.FillData(plot_data);

            //loop over all the fields defined by the format and extract the info we need
            for(auto it = fields.begin(); it != fields.end(); it++)
            {
                std::string field_name = it->get< std::string >();
                if(aformat.contains(field_name))
                {
                    if(aformat[field_name].contains("source_object"))
                    {
                        std::string source_object = aformat[field_name]["source_object"].get< std::string >();
                        if(aformat[field_name].contains("path") && aformat[field_name].contains("type"))
                        {
                            std::string path = aformat[field_name]["path"].get< std::string >();
                            std::string type = aformat[field_name]["type"].get< std::string >();
                            if(source_object == "parameters")
                            {
                                RetrieveParameter(fsum, field_name, paramStore, path, type);
                            }
                            if(source_object == "plot_data")
                            {
                                RetrieveParameter(fsum, field_name, plotData, path, type);
                            }
                            if(source_object == "local_variable")
                            {
                                //only a limited number of these, treat each one individually
                                if(field_name == "fname")
                                {
                                    fsum[field_name] = filename;
                                }
                                if(field_name == "version")
                                {
                                    fsum[field_name] = version;
                                }
                                if(field_name == "extent_no")
                                {
                                    fsum[field_name] = extent_no;
                                }
                            }
                            //deprecated or unavailable objects get a default value
                            if(source_object == "none" && aformat[field_name].contains("default"))
                            {
                                fsum[field_name] = aformat[field_name]["default"];
                            }
                        }
                    }
                }
            }
        }
        else
        {
            msg_error("fringe", "could not read MHO_ObjectTags from: " << filename << eom);
            return false;
        }
        inter.Close();
    }
    else
    {
        msg_error("fringe", "no MHO_ObjectTags object found in file: " << filename << eom);
        return false;
    }

    //extracted ok
    if(ok)
    {
        return true;
    }
    else
    {
        return false;
    }
}

std::string MHO_AFileInfoExtractor::ConvertToAlistRow(const mho_json& data, int version)
{
    int pyear, pday, phour, pmin, psec, syear, sday, shour, smin, ssec = 0;
    int epoch0 = 0;
    int epoch1 = 0;
    char buf[CHAR_BUF_SIZE];

    if(!(version == 5 || version == 6))
    {
        msg_error("fringe", "alist format version: " << version << ", not supported (only 5 or 6)" << eom);
        return "";
    }

    //clear the buffer
    reset_buffer(buf);

    //the 'epoch' parameters are the min,sec of the reference time
    std::string fourfit_reftime = data["epoch"].get< std::string >();
    auto epoch_ldate = hops_clock::to_legacy_hops_date(hops_clock::from_vex_format(fourfit_reftime));
    epoch0 = epoch_ldate.minute;
    epoch1 = epoch_ldate.second;

    auto proc_ldate = hops_clock::to_legacy_hops_date(hops_clock::from_vex_format(data["procdate"].get< std::string >()));
    pyear = proc_ldate.year;
    pday = proc_ldate.day;
    phour = proc_ldate.hour;
    pmin = proc_ldate.minute;
    psec = proc_ldate.second;

    auto time_tag_ldate = hops_clock::to_legacy_hops_date(hops_clock::from_vex_format(data["time_tag"].get< std::string >()));
    syear = time_tag_ldate.year;
    sday = time_tag_ldate.day;
    shour = time_tag_ldate.hour;
    smin = time_tag_ldate.minute;
    ssec = time_tag_ldate.second;

    /* Version 5, Mk4 only, September 99 on */
    if(version == 5)
    {
        sprintf(buf, fformat_v5,
                version,                                      //%ld
                data["root_id"].get< std::string >().c_str(), //%s
                //2
                data["extent_no"].get< int >(),                          //%2d
                data["duration"].get< int64_t >(),                       //%3d
                data["length"].get< int >(),                             //%3d
                data["offset"].get< int >(),                             //%3d
                std::atoi(data["expt_no"].get< std::string >().c_str()), //%4d
                data["scan_id"].get< std::string >().c_str(),            //%8s
                pyear,                                                   //%04d
                pday,                                                    // %03d-
                phour,                                                   // %02d
                pmin,                                                    // %02d
                psec,                                                    // %02d
                syear,                                                   //%04d
                sday,                                                    // %03d-
                shour,                                                   // %02d
                smin,                                                    // %02d
                ssec,                                                    // %02d
                data["scan_offset"].get< int >(),                        //%3d
                data["source"].get< std::string >().c_str(),             //%32s
                data["baseline"].get< std::string >().c_str(),           //2s
                data["quality"].get< std::string >()[0],                 //%c
                data["errcode"].get< std::string >()[0],                 //  %c
                data["freq_code"].get< std::string >()[0],               //%c
                data["no_freq"].get< int >(),                            //%02d
                data["polarization"].get< std::string >().c_str(),       // %2s
                data["lags"].get< int >(),                               //%5d
                data["amp"].get< double >(),                             // %#13.8g
                data["snr"].get< double >(),                             //%#13.8g
                data["resid_phas"].get< double >(),                      // %11.6f
                data["phase_snr"].get< double >(),                       // %#11.6g
                data["datatype"].get< std::string >().c_str(),           //  %2s
                data["sbdelay"].get< double >(),                         //  %+12.9f
                data["mbdelay"].get< double >(),                         // %+12.9f
                data["ambiguity"].get< double >(),                       // %11.9f
                data["delay_rate"].get< double >(),                      // %+11.6f
                data["ref_elev"].get< double >(),                        // %5.2f
                data["rem_elev"].get< double >(),                        // %5.2f
                data["ref_az"].get< double >(),                          // %6.2f
                data["rem_az"].get< double >(),                          // %6.2f
                data["u"].get< double >(),                               // %7.4g
                data["v"].get< double >(),                               // %7.4g
                std::atoi(data["esdesp"].get< std::string >().c_str()),  // %06d
                epoch0,                                                  // %02d
                epoch1,                                                  // %02d
                data["ref_freq"].get< double >(),
                data["total_phas"].get< double >(),    //
                data["total_rate"].get< double >(),    //
                data["total_mbdelay"].get< double >(), //
                data["total_sbresid"].get< double >(), //
                data["srch_cotime"].get< int >(),      //
                data["noloss_cotime"].get< int >()     //
        );
    }

    /* Version 6 EHT Era */
    if(version == 6)
    {
        sprintf(buf, fformat_v6,
                version,                                      //%ld
                data["root_id"].get< std::string >().c_str(), //%s
                //2
                data["extent_no"].get< int >(),                          //%2d
                data["duration"].get< int64_t >(),                       //%3d
                data["length"].get< int >(),                             //%3d
                data["offset"].get< int >(),                             //%3d
                std::atoi(data["expt_no"].get< std::string >().c_str()), //%4d
                data["scan_id"].get< std::string >().c_str(),            //%8s
                pyear,                                                   //%04d
                pday,                                                    // %03d-
                phour,                                                   // %02d
                pmin,                                                    // %02d
                psec,                                                    // %02d
                syear,                                                   //%04d
                sday,                                                    // %03d-
                shour,                                                   // %02d
                smin,                                                    // %02d
                ssec,                                                    // %02d
                data["scan_offset"].get< int >(),                        //%3d
                data["source"].get< std::string >().c_str(),             //%32s
                data["baseline"].get< std::string >().c_str(),           //2s
                data["quality"].get< std::string >()[0],                 //%c
                data["errcode"].get< std::string >()[0],                 //  %c
                data["freq_code"].get< std::string >()[0],               //%c
                data["no_freq"].get< int >(),                            //%02d
                data["polarization"].get< std::string >().c_str(),       // %2s
                data["lags"].get< int >(),                               //%5d
                data["amp"].get< double >(),                             // %#13.8g
                data["snr"].get< double >(),                             //%#13.8g
                data["resid_phas"].get< double >(),                      // %11.6f
                data["phase_snr"].get< double >(),                       // %#11.6g
                data["datatype"].get< std::string >().c_str(),           //  %2s
                data["sbdelay"].get< double >(),                         //  %+12.9f
                data["mbdelay"].get< double >(),                         // %+12.9f
                data["ambiguity"].get< double >(),                       // %11.9f
                data["delay_rate"].get< double >(),                      // %+11.6f
                data["ref_elev"].get< double >(),                        // %5.2f
                data["rem_elev"].get< double >(),                        // %5.2f
                data["ref_az"].get< double >(),                          // %6.2f
                data["rem_az"].get< double >(),                          // %6.2f
                data["u"].get< double >(),                               // %7.4g
                data["v"].get< double >(),                               // %7.4g
                std::atoi(data["esdesp"].get< std::string >().c_str()),  // %06d
                epoch0,                                                  // %02d
                epoch1,                                                  // %02d
                data["ref_freq"].get< double >(),                        //  %9.3f
                data["total_phas"].get< double >(),                      // %10.6f
                data["total_rate"].get< double >(),                      // %11.8f
                data["total_mbdelay"].get< double >(),                   // %13.6f
                data["total_sbresid"].get< double >(),                   // %+9.6f
                data["srch_cotime"].get< int >(),                        // %8d
                data["noloss_cotime"].get< int >(),                      // %8d
                data["ra_hrs"].get< double >(),                          // %+10.6f
                data["dec_deg"].get< double >(),                         // %+10.6f
                data["resid_delay"].get< double >()                      //%+13.10f
        );
    }

    std::string ret_value(buf, strlen(buf));
    return ret_value;
}

void MHO_AFileInfoExtractor::RetrieveParameter(mho_json& obj, const std::string& name, const MHO_ParameterStore& paramStore,
                                               const std::string& path, const std::string& type)
{
    //TODO...eventually replace missing values with default values specified from the format
    par_type ptype = DetermineParameterType(type);
    switch(ptype)
    {
        case int_type:
            if(paramStore.IsPresent(path))
            {
                obj[name] = paramStore.GetAs< int >(path);
            }
            else
            {
                obj[name] = 0;
            }
            break;
        case int64_type:
            if(paramStore.IsPresent(path))
            {
                obj[name] = paramStore.GetAs< int64_t >(path);
            }
            else
            {
                obj[name] = 0;
            }
            break;
        case double_type:
            if(paramStore.IsPresent(path))
            {
                obj[name] = paramStore.GetAs< double >(path);
            }
            else
            {
                obj[name] = 0;
            }
            break;
        case string_type:
            if(paramStore.IsPresent(path))
            {
                obj[name] = paramStore.GetAs< std::string >(path);
            }
            else
            {
                obj[name] = "";
            }
            break;
        case bool_type:
            if(paramStore.IsPresent(path))
            {
                obj[name] = paramStore.GetAs< bool >(path);
            }
            else
            {
                obj[name] = 0;
            }
            break;
    };
}

par_type MHO_AFileInfoExtractor::DetermineParameterType(std::string etype)
{
    if(etype == "int")
    {
        return int_type;
    }
    if(etype == "int64_t")
    {
        return int64_type;
    }
    if(etype == "double")
    {
        return double_type;
    }
    if(etype == "string")
    {
        return string_type;
    }
    if(etype == "bool")
    {
        return bool_type;
    }
    return unknown_type;
}

std::string MHO_AFileInfoExtractor::RetrieveParameterAsString(const mho_json& obj, const std::string& name,
                                                              const std::string& type, const std::string& pformat)
{
    par_type ptype = DetermineParameterType(type);
    switch(ptype)
    {
        case int_type:
            {
                int value = 0;
                if(obj.contains(name))
                {
                    value = obj[name].get< int >();
                }
                return ConvertToString(value, pformat);
            }
            break;
        case int64_type:
            {
                int64_t value = 0;
                if(obj.contains(name))
                {
                    value = obj[name].get< int64_t >();
                }
                return ConvertToString(value, pformat);
            }
            break;
        case double_type:
            {
                double value = 0;
                if(obj.contains(name))
                {
                    value = obj[name].get< double >();
                }
                return ConvertToString(value, pformat);
            }
            break;
        case string_type:
            {
                std::string value;
                if(obj.contains(name))
                {
                    value = obj[name].get< std::string >();
                }
                return ConvertToString(value, pformat);
            }
            break;
        case bool_type:
            {
                bool value;
                if(obj.contains(name))
                {
                    value = obj[name].get< bool >();
                }
                return ConvertToString(value, pformat);
            }
            break;
    };

    //if we made it here just return empty string
    return std::string("");
}

} // namespace hops
