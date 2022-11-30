#include "MHO_MK4FringeInterface.hh"

#include "MHO_MultiTypeMap.hh"
#include <array>

namespace hops
{

template< typename XType, size_t N>
std::array<XType, N> create_and_fill_array(XType values[N])
{
    std::array<XType, N> arr;
    for(size_t i=0; i<N; i++)
    {
        arr[i] = values[i];
    }
    return arr;
}


MHO_MK4FringeInterface::MHO_MK4FringeInterface():
    fHaveFringe(false)
{

}

MHO_MK4FringeInterface::~MHO_MK4FringeInterface()
{
    clear_mk4fringe(&fFringe);
}

void
MHO_MK4FringeInterface::ReadFringeFile(const std::string& filename)
{
    if(fHaveFringe)
    {
        clear_mk4fringe(&fFringe);
    }

    std::string fname = filename;
    int retval = read_mk4fringe( const_cast<char*>(fname.c_str()), &fFringe );
    if(retval == 0)
    {
        fHaveFringe = true;
    }
    else
    {
        fHaveFringe = false;
    }
}

void
MHO_MK4FringeInterface::ExportFringeFile()
{
    if(fHaveFringe)
    {
        //want to dump the information in the type_200 through type_230 objects
        //for now just do the POD data types

        MHO_MultiTypeMap< std::string, int, short, float, double, std::array<double, 4>, std::string> _m;

        // type_200
        _m.Insert( std::string("type200.record_id"), std::string(fFringe.t200->record_id));
        _m.Insert( std::string("type200.version_no"), std::string(fFringe.t200->version_no));
        _m.Insert( std::string("type200.unused1"), std::string(fFringe.t200->unused1));
        _m.Insert( std::string("type200.software_rev"), fFringe.t200->software_rev);
        _m.Insert( std::string("type200.expt_no"), fFringe.t200->expt_no);
        _m.Insert( std::string("type200.exper_name"), std::string(fFringe.t200->exper_name));
        _m.Insert( std::string("type200.scan_name"), std::string(fFringe.t200->scan_name));
        _m.Insert( std::string("type200.correlator"), std::string(fFringe.t200->correlator));
        _m.Insert( std::string("type200.scantime.year"), fFringe.t200.scantime->year);
        _m.Insert( std::string("type200.scantime.day"), fFringe.t200.scantime->day);
        _m.Insert( std::string("type200.scantime.hour"), fFringe.t200.scantime->hour);
        _m.Insert( std::string("type200.scantime.minute"), fFringe.t200.scantime->minute);
        _m.Insert( std::string("type200.scantime.second"), fFringe.t200.scantime->second);
        _m.Insert( std::string("type200.start_offset"), fFringe.t200->start_offset);
        _m.Insert( std::string("type200.stop_offset"), fFringe.t200->stop_offset);
        _m.Insert( std::string("type200.corr_date.year"), fFringe.t200.corr_date->year);
        _m.Insert( std::string("type200.corr_date.day"), fFringe.t200.corr_date->day);
        _m.Insert( std::string("type200.corr_date.hour"), fFringe.t200.corr_date->hour);
        _m.Insert( std::string("type200.corr_date.minute"), fFringe.t200.corr_date->minute);
        _m.Insert( std::string("type200.corr_date.second"), fFringe.t200.corr_date->second);
        _m.Insert( std::string("type200.fourfit_date.year"), fFringe.t200.fourfit_date->year);
        _m.Insert( std::string("type200.fourfit_date.day"), fFringe.t200.fourfit_date->day);
        _m.Insert( std::string("type200.fourfit_date.hour"), fFringe.t200.fourfit_date->hour);
        _m.Insert( std::string("type200.fourfit_date.minute"), fFringe.t200.fourfit_date->minute);
        _m.Insert( std::string("type200.fourfit_date.second"), fFringe.t200.fourfit_date->second);

        // type_201 data
        _m.Insert( std::string("type201.record_id"), std::string(fFringe.t201->record_id));
        _m.Insert( std::string("type201.version_no"), std::string(fFringe.t201->version_no));
        _m.Insert( std::string("type201.unused1"), std::string(fFringe.t201->unused1));
        _m.Insert( std::string("type201.source"), std::string(fFringe.t201->source));
        _m.Insert( std::string("type201.coord.ra_hrs"), fFringe.t201.coord->ra_hrs);
        _m.Insert( std::string("type201.coord.ra_mins"), fFringe.t201.coord->ra_mins);
        _m.Insert( std::string("type201.coord.ra_secs"), fFringe.t201.coord->ra_secs);
        _m.Insert( std::string("type201.coord.dec_degs"), fFringe.t201.coord->dec_degs);
        _m.Insert( std::string("type201.coord.dec_mins"), fFringe.t201.coord->dec_mins);
        _m.Insert( std::string("type201.coord.dec_secs"), fFringe.t201.coord->dec_secs);
        _m.Insert( std::string("type201.epoch"), fFringe.t201->epoch );
        _m.Insert( std::string("type201.unused2"), std::string(fFringe.t201->unused2) );
        _m.Insert( std::string("type200.coord_date.year"), fFringe.t200.coord_date->year);
        _m.Insert( std::string("type200.coord_date.day"), fFringe.t200.coord_date->day);
        _m.Insert( std::string("type200.coord_date.hour"), fFringe.t200.coord_date->hour);
        _m.Insert( std::string("type200.coord_date.minute"), fFringe.t200.coord_date->minute);
        _m.Insert( std::string("type200.coord_date.second"), fFringe.t200.coord_date->second);
        _m.Insert( std::string("type201.ra_rate"), fFringe.t201->ra_rate );
        _m.Insert( std::string("type201.dec_rate"), fFringe.t201->dec_rate );
        _m.Insert( std::string("type201.pulsar_phase"), create_and_fill_array<double, 4>(fFringe.t201->pulsar_phase) );
        _m.Insert( std::string("type201.pulsar_epoch"), fFringe.t201->pulsar_epoch );
        _m.Insert( std::string("type201.dispersion"), fFringe.t201->dispersion );


    }
}

void MHO_MK4FringeInterface::ExportFringeFileToJSON(){
    // Call typ200 functions here

    // Print out fringe file data
    cout << fFringe;

    //  
}




}
