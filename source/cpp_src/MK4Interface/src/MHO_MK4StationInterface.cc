#include "MHO_MK4StationInterface.hh"

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
    fHaveVex(false),
    fStation(nullptr),
    fVex(nullptr)
{

    fVex = (struct vex *) calloc ( 1, sizeof(struct vex) );
    fStation = (struct mk4_sdata *) calloc ( 1, sizeof(struct mk4_sdata) );
    fNCoeffs = 0;
    fNIntervals = 0;
    fNChannels = 0;
    fNCoord = 0;
}

MHO_MK4StationInterface::~MHO_MK4StationInterface()
{
    clear_mk4sdata(fStation);
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
        //(2) phase spline polynomial coeff (type_302)
        //(3) parallatic angle spline coeff (type_303)
        //(4) uvw-coords spline coeff (type_303)
        //(5) phase-cal data (type_309)

        //We need to determine 4 things:
        //the number of channels
        //the number of intervals
        //the number of spline coefficients
        fNCoord = NCOORD; //delay, phase, az, el, par-angle, u, v, w
        fNCoeffs = NCOEFF; // hard-coded value in the mk4 type_301, 302, 303s for max spline coeff
        fNIntervals = 0;
        fNChannels = 0;
        DetermineDataDimensions();

        std::size_t st_dim[STATION_NDIM] = {fNCoord, fNChannels, fNIntervals, fNCoeffs};
        st_data = new station_coord_type(st_dim);

        std::get<COORD_AXIS>(*st_data)[0] = std::string("delay");
        std::get<COORD_AXIS>(*st_data)[1] = std::string("phase");
        std::get<COORD_AXIS>(*st_data)[2] = std::string("azimuth");
        std::get<COORD_AXIS>(*st_data)[3] = std::string("elevation");
        std::get<COORD_AXIS>(*st_data)[4] = std::string("parallactic_angle");
        std::get<COORD_AXIS>(*st_data)[5] = std::string("u");
        std::get<COORD_AXIS>(*st_data)[6] = std::string("v");
        std::get<COORD_AXIS>(*st_data)[7] = std::string("w");

        //now lets extract the spline data from each channel
        for(std::size_t ch=0; ch<fNChannels; ch++)
        {
            std::string chan_id = std::string(fStation->model[ch].chan_id);
            if(chan_id.size() != 0)
            {
                std::get<CHAN_AXIS>(*st_data)[ch] = chan_id;
                for(std::size_t sp=0; sp<fNIntervals; sp++)
                {
                    std::get<INTERVAL_AXIS>(*st_data)[sp] = sp;

                    type_301* t301 = fStation->model[ch].t301[sp]; //delay
                    type_302* t302 = fStation->model[ch].t302[sp]; //phase
                    type_303* t303 = fStation->model[ch].t303[sp]; //az,el,par,u,v,w
                    if( t301 != nullptr && t302 != nullptr && t302 != nullptr)
                    {
                        if( t301->interval != sp){msg_error("mk4interface", "spline interval mis-match." << eom);};
                        if( t302->interval != sp){msg_error("mk4interface", "spline interval mis-match." << eom);};
                        if( t303->interval != sp){msg_error("mk4interface", "spline interval mis-match." << eom);};

                        for(std::size_t cf=0; cf<fNCoeffs; cf++)
                        {
                            std::get<COEFF_AXIS>(*st_data)[cf] = cf;
                            st_data->at(0, ch, sp, cf) = t301->delay_spline[cf];
                            st_data->at(1, ch, sp, cf) = t302->phase_spline[cf];
                            st_data->at(2, ch, sp, cf) = t303->azimuth[cf];
                            st_data->at(3, ch, sp, cf) = t303->elevation[cf];
                            st_data->at(4, ch, sp, cf) = t303->parallactic_angle[cf];
                            st_data->at(5, ch, sp, cf) = t303->u[cf];
                            st_data->at(6, ch, sp, cf) = t303->v[cf];
                            st_data->at(7, ch, sp, cf) = t303->w[cf];
                        }
                    }
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
        fVex = (struct vex *) calloc ( 1, sizeof(struct vex) );
        fHaveVex = false;
    }

    std::string tmp_key(""); //use empty key for now
    std::string fname = fVexFile;
    int retval = get_vex( const_cast<char*>(fname.c_str() ),
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


void
MHO_MK4StationInterface::DetermineDataDimensions()
{
    fNIntervals = fStation->t300->nsplines;
    msg_debug("mk4interface", "Number of spline intervals = "<< fNIntervals << eom);
    for(std::size_t i=0; i<MAXFREQ; i++)
    {
        //loop through all the channel id's and count the ones which are unlabelled
        std::string channel_id = std::string( fStation->model[i].chan_id );
        if(channel_id.size() != 0){fNChannels++;}
    }
    //for now ignore p-cal data
    //we will have to determine p-cal data dims here later
}


}//end of namespace
