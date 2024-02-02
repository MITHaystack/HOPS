#include "MHO_MK4StationInterface.hh"
#include "MHO_LegacyDateConverter.hh"

#include <vector>
#include <cstdlib>
#include <cstring>
#include <complex>
#include <set>
#include <algorithm>

//mk4 IO library
#ifndef HOPS3_USE_CXX
extern "C"
{
#endif
    #include "mk4_records.h"
    #include "mk4_data.h"
    #include "mk4_dfio.h"
    #include "mk4_vex.h"
#ifndef HOPS3_USE_CXX
}
#endif

namespace hops
{


MHO_MK4StationInterface::MHO_MK4StationInterface():
    fHaveStation(false),
    fStation(nullptr)
{
    fStation = (struct mk4_sdata *) calloc ( 1, sizeof(struct mk4_sdata) );
    fNCoeffs = 0;
    fNIntervals = 0;
    fNCoord = 0;
}

MHO_MK4StationInterface::~MHO_MK4StationInterface()
{
    clear_mk4sdata(fStation);
    free(fStation);
}

station_coord_type*
MHO_MK4StationInterface::ExtractStationFile()
{

    ReadStationFile();

    station_coord_type* st_data = nullptr;

    if(fHaveStation)
    {
        //first thing we have to do is figure out the data dimensions
        //the items stored in the mk4sdata objects are mainly:
        //(0) meta day about the spline model (type_300)
        //(1) delay spline polynomial coeff (type_301)
        //(2) parallatic angle spline coeff (type_303)
        //(3) uvw-coords spline coeff (type_303)
        //(4) phase-cal data (type_309) -- no yet supported here

        //we do not export the channel-dependent phase spline data e.g.
        //phase spline polynomial coeff (type_302)
        //as this can be constructed from the channel freq * delay spline

        fNCoord = NCOORD; //delay, az, el, par-angle, u, v, w (no phase spline)
        fNCoeffs = NCOEFF; //hard-coded value in the mk4 type_301, 303, etc. for max spline coeff
        fNIntervals = fStation->t300->nsplines; //the number of spline intervals

        st_data = new station_coord_type();
        st_data->Resize(fNCoord, fNIntervals, fNCoeffs);
        st_data->ZeroArray();

        std::get<COORD_AXIS>(*st_data)[0] = "delay";
        std::get<COORD_AXIS>(*st_data)[1] = "azimuth";
        std::get<COORD_AXIS>(*st_data)[2] = "elevation";
        std::get<COORD_AXIS>(*st_data)[3] = "parallactic_angle";
        std::get<COORD_AXIS>(*st_data)[4] = "u";
        std::get<COORD_AXIS>(*st_data)[5] = "v";
        std::get<COORD_AXIS>(*st_data)[6] = "w";

        //extract some basics from the type_300 
        type_300* t300 = fStation->t300;
        double spline_interval = t300->model_interval;

        //convert the legacy date struct to a cannonical date/time-stamp string
        struct date model_start = t300->model_start;
        legacy_hops_date ldate;
        ldate.year = model_start.year;
        ldate.day = model_start.day;
        ldate.hour = model_start.hour;
        ldate.minute = model_start.minute;
        ldate.second = model_start.second;
        std::string model_start_date = MHO_LegacyDateConverter::ConvertToVexFormat(ldate);

        //retrieve the station name/id
        std::string mk4_id = getstr(&(t300->id), 1);
        std::string station_id = getstr(t300->intl_id, 2);
        std::string station_name = getstr(t300->name, 32);

        //tag the station data structure with all the meta data from the type_300
        st_data->Insert(std::string("name"), std::string("station_data"));
        st_data->Insert(std::string("station_name"), station_name);
        st_data->Insert(std::string("station_mk4id"), mk4_id);
        st_data->Insert(std::string("station_code"), station_id);
        st_data->Insert(std::string("model_start"), model_start_date);
        st_data->Insert(std::string("nsplines"), fStation->t300->nsplines);
        st_data->Insert(std::string("model_interval"), spline_interval);

        //with the exception of the type_302s, the spline data is the same from each channel, so just use ch=0
        std::size_t ch = 0;
        for(std::size_t sp=0; sp<fNIntervals; sp++)
        {
            std::get<INTERVAL_AXIS>(*st_data)[sp] = sp*spline_interval; //seconds since start
            type_301* t301 = fStation->model[ch].t301[sp]; //delay
            type_303* t303 = fStation->model[ch].t303[sp]; //az,el,par,u,v,w
            if( t301 != nullptr && t303 != nullptr)
            {
                if( t301->interval != sp){msg_error("mk4interface", "spline interval mis-match." << eom);};
                if( t303->interval != sp){msg_error("mk4interface", "spline interval mis-match." << eom);};

                for(std::size_t cf=0; cf<fNCoeffs; cf++)
                {
                    std::get<COEFF_AXIS>(*st_data)[cf] = cf; //polynomial power of this term
                    st_data->at(0, sp, cf) = t301->delay_spline[cf];
                    st_data->at(1, sp, cf) = t303->azimuth[cf];
                    st_data->at(2, sp, cf) = t303->elevation[cf];
                    st_data->at(3, sp, cf) = t303->parallactic_angle[cf];
                    st_data->at(4, sp, cf) = t303->u[cf];
                    st_data->at(5, sp, cf) = t303->v[cf];
                    st_data->at(6, sp, cf) = t303->w[cf];
                }
            }
        }


        //now deal with the type_309 pcal data 
        int n309 = fStation->n309;
        if(n309 != 0)
        {
            type_309** t309 = fStation->t309;

            for(int i=0; i < n309; i++)
            {
                int su = t309[i]->su;
                int ntones = t309[i]->ntones;
                double rot = t309[i]->rot;
                double acc_period = t309[i]->acc_period;
                std::cout<<"t309 @ "<<i<<" = "<<su<<", "<<ntones<<", "<<rot<<", "<<acc_period<<std::endl;

                for(int ch=0; ch < 64; ch++)
                {
                    std::string ch_name( &(t309[i]->chan[ch].chan_name[0]), 8);
                    std::cout<<"chan name = "<<ch_name<<std::endl;

                    for(int ti=0; ti < 64; ti++)
                    {
                        std::cout<<"tone"<<ti<<" = ("<<t309[i]->chan[ch].acc[ti][0]<<", "<<t309[i]->chan[ch].acc[ti][1]<<std::endl;
                    }

                }
                

            }

                // char        record_id[3];           // Standard 3-digit id
                // char        version_no[2];          // Standard 2-digit version #
                // char        unused1[3]; 
                // int         su;                     // SU
                // int         ntones;                 // number of tones [0..64]
                // double      rot;                    // ROT at start of AP
                // double      acc_period;             // in secs
                // struct ch1_tag
                //   {
                //   char      chan_name[8];
                //   double    freq;                   // tone frequency in Hz
                //   U32       acc[64][2];             // accumulators for 64 freqs x 2 quads (C..S)
                //   } chan[64];
                // };
        }

    }

    return st_data;

}


void MHO_MK4StationInterface::ReadStationFile()
{
    if(fHaveStation)
    {
        msg_debug("mk4interface", "Clearing a previously exisiting station data struct."<< eom);
        clear_mk4sdata(fStation);
        fStation = nullptr;
        fHaveStation = false;
    }

    //have to copy fStationFile for const_cast, as mk4 lib doesn't respect const
    std::string fname = fStationFile;
    int retval = read_mk4sdata( const_cast<char*>(fname.c_str()), fStation );
    if(retval == 0)
    {
        fHaveStation = true;
        msg_debug("mk4interface", "Successfully read station data file."<< fStationFile << eom);
    }
    else
    {
        fHaveStation = false;
        msg_debug("mk4interface", "Failed to read station data file: "<< fStationFile << ", error value: "<< retval << eom);
    }
}


}//end of namespace
