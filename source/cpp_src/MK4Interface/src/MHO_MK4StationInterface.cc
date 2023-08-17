#include "MHO_MK4StationInterface.hh"
#include "MHO_Clock.hh"

#include <vector>
#include <cstdlib>
#include <cstring>
#include <complex>
#include <set>
#include <algorithm>

namespace mk4
{

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

}

namespace hops
{


MHO_MK4StationInterface::MHO_MK4StationInterface():
    fHaveStation(false),
    fHaveVex(false),
    fStation(nullptr),
    fVex(nullptr)
{

    fVex = (struct mk4::vex *) calloc ( 1, sizeof(struct mk4::vex) );
    fStation = (struct mk4::mk4_sdata *) calloc ( 1, sizeof(struct mk4::mk4_sdata) );
    fNCoeffs = 0;
    fNIntervals = 0;
    fNCoord = 0;
}

MHO_MK4StationInterface::~MHO_MK4StationInterface()
{
    mk4::clear_mk4sdata(fStation);
    free(fStation);
    free(fVex);
}

station_coord_type*
MHO_MK4StationInterface::ExtractStationFile()
{

    ReadStationFile();
    ReadVexFile();

    station_coord_type* st_data = nullptr;

    if(fHaveStation && fHaveVex)
    {
        //first thing we have to do is figure out the data dimensions
        //the items stored in the mk4sdata objects are mainly:
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
        mk4::type_300* t300 = fStation->t300;
        double spline_interval = t300->model_interval;

        //TODO FIXME! we need to convert this data struct to a cannonical date/time-stamp class
        struct mk4::date model_start = t300->model_start;
        legacy_hops_date ldate;
        ldate.year = model_start.year;
        ldate.day = model_start.day;
        ldate.hour = model_start.hour;
        ldate.minute = model_start.minute;
        ldate.second = model_start.second;
        
        //std::cout<<"hops time-point converted from legacy hops-date-struct: "<<std::endl;
        std::cout<<"year = "<<ldate.year<<std::endl;
        std::cout<<"date = "<<ldate.day<<std::endl;
        std::cout<<"hour = "<<ldate.hour<<std::endl;
        std::cout<<"mins = "<<ldate.minute<<std::endl;
        std::cout<<"secs = "<< std::setprecision(9) <<ldate.second<<std::endl;
        
        auto mstart = hops_clock::from_legacy_hops_date(ldate);
        
        std::cout<<"to hops date in iso-8601 format: "<<hops_clock::to_iso8601_format(mstart)<<std::endl;
        
        //with the exception of the type_302s, the spline data is the same from each channel, so just use ch=0
        std::size_t ch = 0;
        for(std::size_t sp=0; sp<fNIntervals; sp++)
        {
            std::get<INTERVAL_AXIS>(*st_data)[sp] = sp*spline_interval; //seconds since start
            mk4::type_301* t301 = fStation->model[ch].t301[sp]; //delay
            mk4::type_303* t303 = fStation->model[ch].t303[sp]; //az,el,par,u,v,w
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
    }

    return st_data;

}


//corel and vex file members
void MHO_MK4StationInterface::ReadStationFile()
{
    if(fHaveStation)
    {
        msg_debug("mk4interface", "Clearing a previously exisiting station data struct."<< eom);
        mk4::clear_mk4sdata(fStation);
        fStation = nullptr;
        fHaveStation = false;
    }

    //have to copy fStationFile for const_cast, as mk4 lib doesn't respect const
    std::string fname = fStationFile;
    int retval = mk4::read_mk4sdata( const_cast<char*>(fname.c_str()), fStation );
    if(retval == 0)
    {
        fHaveStation = true;
        msg_debug("mk4interface", "Failed to read station data file: "<< fStationFile << ", error value: "<< retval << eom);
    }
    else
    {
        fHaveStation = false;
        msg_debug("mk4interface", "Successfully read station data file."<< fStationFile << eom);
    }
}


void MHO_MK4StationInterface::ReadVexFile()
{
    if(fHaveVex)
    {
        msg_debug("mk4interface", "Clearing a previously exisiting vex struct."<< eom);
        free(fVex);
        fVex = (struct mk4::vex *) calloc ( 1, sizeof(struct mk4::vex) );
        fHaveVex = false;
    }

    std::string tmp_key(""); //use empty key for now
    std::string fname = fVexFile;
    int retval = mk4::get_vex( const_cast<char*>(fname.c_str() ),
                          OVEX | EVEX | IVEX | LVEX ,
                          const_cast<char*>(tmp_key.c_str() ), fVex);

    if(retval !=0 )
    {
        fHaveVex = false;
        msg_debug("mk4interface", "Failed to read vex file: " << fVexFile << ", error value: "<< retval << eom);
    }
    else
    {
        fHaveVex = true;
        msg_debug("mk4interface", "Successfully read vex file."<< fVexFile << eom);
    }
}



}//end of namespace
