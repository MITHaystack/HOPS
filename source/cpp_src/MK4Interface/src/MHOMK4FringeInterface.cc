#include "MHOMK4FringeInterface.hh"

#include "MHOMultiTypeMap.hh"
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


MHOMK4FringeInterface::MHOMK4FringeInterface():
    fHaveFringe(false)
{

}

MHOMK4FringeInterface::~MHOMK4FringeInterface()
{
    clear_mk4fringe(&fFringe);
}

void
MHOMK4FringeInterface::ReadFringeFile(const std::string& filename)
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
MHOMK4FringeInterface::ExportFringeFile()
{
    if(fHaveFringe)
    {
        //want to dump the information in the type_200 through type_230 objects
        //for now just do the POD data types

        MHOMultiTypeMap< std::string, int, short, float, double, std::array<double, 4>, std::string> _m;

        //type_200
        _m.Insert( std::string("type200.record_id"), std::string(fFringe.t200->record_id) );
        _m.Insert( std::string("type200.version_no"), std::string(fFringe.t200->version_no) );
        _m.Insert( std::string("type200.unused1"), std::string(fFringe.t200->unused1) );
        //_m.Insert( std::string("type200.software_rev"), std::string(fFringe.t200->software_rev) );
        _m.Insert( std::string("type200.expt_no"), fFringe.t200->expt_no );
        _m.Insert( std::string("type200.exper_name"), std::string(fFringe.t200->exper_name) );
        _m.Insert( std::string("type200.scan_name"), std::string(fFringe.t200->scan_name) );
        _m.Insert( std::string("type200.correlator"), std::string(fFringe.t200->correlator) );
        //_m.Insert( std::string("type200.scantime"), std::string(fFringe.t200->scantime) );
        _m.Insert( std::string("type200.start_offset"), fFringe.t200->start_offset );
        _m.Insert( std::string("type200.stop_offset"), fFringe.t200->stop_offset );
        //_m.Insert( std::string("type200.corr_date"), std::string(fFringe.t200->corr_date) );
        //_m.Insert( std::string("type200.fourfit_date"), std::string(fFringe.t200->fourfit_date) );
        //_m.Insert( std::string("type200.frt"), std::string(fFringe.t200->frt) );

        //type 201
        _m.Insert( std::string("type201.record_id"), std::string(fFringe.t201->record_id) );
        _m.Insert( std::string("type201.version_no"), std::string(fFringe.t201->version_no) );
        _m.Insert( std::string("type201.unused1"), std::string(fFringe.t201->unused1) );
        _m.Insert( std::string("type201.source"), std::string(fFringe.t201->source) );
        //_m.Insert( std::string("type201.coord"), fFringe.t201->coord );
        _m.Insert( std::string("type201.epoch"), fFringe.t201->epoch );
        _m.Insert( std::string("type201.unused2"), std::string(fFringe.t201->unused2) );
        //_m.Insert( std::string("type201.coord_date"), std::string(fFringe.t201->coord_date) );
        _m.Insert( std::string("type201.ra_rate"), fFringe.t201->ra_rate );
        _m.Insert( std::string("type201.dec_rate"), fFringe.t201->dec_rate );
        _m.Insert( std::string("type201.pulsar_phase"), create_and_fill_array<double, 4>(fFringe.t201->pulsar_phase) );
        _m.Insert( std::string("type201.pulsar_epoch"), fFringe.t201->pulsar_epoch );
        _m.Insert( std::string("type201.dispersion"), fFringe.t201->dispersion );


    }
}




}
