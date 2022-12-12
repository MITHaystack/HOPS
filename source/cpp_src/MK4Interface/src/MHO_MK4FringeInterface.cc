#include "MHO_MK4FringeInterface.hh"
#include "MHO_MK4Type203Converter.hh"
#include "MHO_MK4Type205Converter.hh"

#include "MHO_MultiTypeMap.hh"
#include <array>
#include <string>

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
        _m.Insert( std::string("type201.unused2"), std::string(fFringe.t201->unused2));
        _m.Insert( std::string("type201.coord_date.year"), fFringe.t201.coord_date->year);
        _m.Insert( std::string("type201.coord_date.day"), fFringe.t201.coord_date->day);
        _m.Insert( std::string("type201.coord_date.hour"), fFringe.t201.coord_date->hour);
        _m.Insert( std::string("type201.coord_date.minute"), fFringe.t201.coord_date->minute);
        _m.Insert( std::string("type201.coord_date.second"), fFringe.t201.coord_date->second);
        _m.Insert( std::string("type201.ra_rate"), fFringe.t201->ra_rate );
        _m.Insert( std::string("type201.dec_rate"), fFringe.t201->dec_rate );
        _m.Insert( std::string("type201.pulsar_phase"), create_and_fill_array<double, 4>(fFringe.t201->pulsar_phase) );
        _m.Insert( std::string("type201.pulsar_epoch"), fFringe.t201->pulsar_epoch );
        _m.Insert( std::string("type201.dispersion"), fFringe.t201->dispersion );

        // type_202 data
        _m.Insert( std::string("type202.record_id"), std::string(fFringe.t202->record_id));
        _m.Insert( std::string("type202.version_no"), std::string(fFringe.t202->version_no));
        _m.Insert( std::string("type202.unused1"), std::string(fFringe.t202->unused1));
        _m.Insert( std::string("type202.baseline"), std::string(fFringe.t202->baseline));
        _m.Insert( std::string("type202.ref_intl_id"), std::string(fFringe.t202->ref_intl_id));
        _m.Insert( std::string("type202.rem_intl_id"), std::string(fFringe.t202->rem_intl_id));
        _m.Insert( std::string("type202.ref_name"), std::string(fFringe.t202->ref_name));
        _m.Insert( std::string("type202.rem_name"), std::string(fFringe.t202->rem_name));
        _m.Insert( std::string("type202.ref_tape"), std::string(fFringe.t202->ref_tape));
        _m.Insert( std::string("type202.rem_tape"), std::string(fFringe.t202->rem_tape));
        _m.Insert( std::string("type202.nlags"), fFringe.t202->nlags);
        _m.Insert( std::string("type202.ref_xpos"), fFringe.t202->ref_xpos);
        _m.Insert( std::string("type202.rem_xpos"), fFringe.t202->rem_xpos);
        _m.Insert( std::string("type202.rem_ypos"), fFringe.t202->rem_ypos);
        _m.Insert( std::string("type202.ref_ypos"), fFringe.t202->ref_ypos);
        _m.Insert( std::string("type202.ref_zpos"), fFringe.t202->ref_zpos);
        _m.Insert( std::string("type202.rem_zpos"), fFringe.t202->rem_zpos);
        _m.Insert( std::string("type202.u"), fFringe.t202->u);
        _m.Insert( std::string("type202.v"), fFringe.t202->v);
        _m.Insert( std::string("type202.uf"), fFringe.t202->uf);
        _m.Insert( std::string("type202.vf"), fFringe.t202->vf);
        _m.Insert( std::string("type202.ref_clock"), fFringe.t202->ref_clock);
        _m.Insert( std::string("type202.rem_clock"), fFringe.t202->rem_clock);
        _m.Insert( std::string("type202.ref_clockrate"), fFringe.t202->ref_clockrate);
        _m.Insert( std::string("type202.rem_clockrate"), fFringe.t202->rem_clockrate);
        _m.Insert( std::string("type202.ref_idelay"), fFringe.t202->ref_idelay);
        _m.Insert( std::string("type202.rem_idelay"), fFringe.t202->rem_idelay);
        _m.Insert( std::string("type202.ref_zdelay"), fFringe.t202->ref_zdelay);
        _m.Insert( std::string("type202.rem_zdelay"), fFringe.t202->rem_zdelay);
        _m.Insert( std::string("type202.ref_elev"), fFringe.t202->ref_elev);
        _m.Insert( std::string("type202.rem_elev"), fFringe.t202->rem_elev);
        _m.Insert( std::string("type202.ref_az"), fFringe.t202->ref_az);
        _m.Insert( std::string("type202.rem_az"), fFringe.t202->rem_az);

        // type_203 data
        _m.Insert( std::string("type203.record_id"), std::string(fFringe.t203->record_id));
        _m.Insert( std::string("type203.version_no"), std::string(fFringe.t203->version_no));
        for (int i = 0; i < NUMBEROFCHANNELS; i++) {
            _m.Insert( std::string(std::format("type203.channels[{}].index", i)), fFringe.t203.channels[i]->index);
            _m.Insert( std::string(std::format("type203.channels[{}].sample_rate", i)), fFringe.t203.channels[i]->sample_rate);
            _m.Insert( std::string(std::format("type203.channels[{}].refsb ", i)), fFringe.t203.channels[i]->refsb);
            _m.Insert( std::string(std::format("type203.channels[{}].remsb", i)), fFringe.t203.channels[i]->remsb);
            _m.Insert( std::string(std::format("type203.channels[{}].rempol", i)), fFringe.t203.channels[i]->rempol);
            _m.Insert( std::string(std::format("type203.channels[{}].ref_freq", i)), fFringe.t203.channels[i]->ref_freq);
            _m.Insert( std::string(std::format("type203.channels[{}].rem_freq", i)), fFringe.t203.channels[i]->rem_freq);
            _m.Insert( std::string(std::format("type203.channels[{}].ref_chan_id", i)),  std::string(fFringe.t203.channels[i]->ref_chan_id));
            _m.Insert( std::string(std::format("type203.channels[{}].rem_chan_id", i)),  std::string(fFringe.t203.channels[i]->rem_chan_id));
        }

        // type_204 data
        _m.Insert( std::string("type204.record_id"), std::string(fFringe.t204->record_id));
        _m.Insert( std::string("type204.version_no"), std::string(fFringe.t204->version_no));
        _m.Insert( std::string("type204.ff_version"), create_and_fill_array<short, 2>(fFringe.t204->ff_version) );
        _m.Insert( std::string("type204.platform"), std::string(fFringe.t204->platform));
        _m.Insert( std::string("type204.control_file"), std::string(fFringe.t204->control_file));
        _m.Insert( std::string("type204.ffcf_date.year"), fFringe.t204.ffcf_date->year);
        _m.Insert( std::string("type204.ffcf_date.day"), fFringe.t204.ffcf_date->day);
        _m.Insert( std::string("type204.ffcf_date.hour"), fFringe.t204.ffcf_date->hour);
        _m.Insert( std::string("type204.ffcf_date.minute"), fFringe.t204.ffcf_date->minute);
        _m.Insert( std::string("type204.ffcf_date.second"), fFringe.t204.ffcf_date->second);
        _m.Insert( std::string("type204.override"), std::string(fFringe.t204->override));

        // type_205 data
        _m.Insert( std::string("type205.record_id"), std::string(fFringe.t205->record_id));
        _m.Insert( std::string("type205.version_no"), std::string(fFringe.t205->version_no));
        _m.Insert( std::string("type205.unused1"), std::string(fFringe.t205->unused1));
        _m.Insert( std::string("type205.utc_central.year"), fFringe.t205.utc_central->year);
        _m.Insert( std::string("type205.utc_central.day"), fFringe.t205.utc_central->day);
        _m.Insert( std::string("type205.utc_central.hour"), fFringe.t205.utc_central->hour);
        _m.Insert( std::string("type205.utc_central.minute"), fFringe.t205.utc_central->minute);
        _m.Insert( std::string("type205.utc_central.second"), fFringe.t205.utc_central->second);
        _m.Insert( std::string("type205.offset"), fFringe.t205->offset);
        _m.Insert( std::string("type205.ffmode"), std::string(fFringe.t205->ffmode));
        _m.Insert( std::string("type205.search"), create_and_fill_array<float, 6>(fFringe.t205->search));
        _m.Insert( std::string("type205.filter"), create_and_fill_array<float, 8>(fFringe.t205->filter));
        _m.Insert( std::string("type205.start.year"), fFringe.t205.start->year);
        _m.Insert( std::string("type205.start.day"), fFringe.t205.start->day);
        _m.Insert( std::string("type205.start.hour"), fFringe.t205.start->hour);
        _m.Insert( std::string("type205.start.minute"), fFringe.t205.start->minute);
        _m.Insert( std::string("type205.start.second"), fFringe.t205.start->second);
        _m.Insert( std::string("type205.stop.year"), fFringe.t205.stop->year);
        _m.Insert( std::string("type205.stop.day"), fFringe.t205.stop->day);
        _m.Insert( std::string("type205.stop.hour"), fFringe.t205.stop->hour);
        _m.Insert( std::string("type205.stop.minute"), fFringe.t205.stop->minute);
        _m.Insert( std::string("type205.stop.second"), fFringe.t205.stop->second);
        _m.Insert( std::string("type205.ref_freq"), fFringe.t205->ref_freq);
        for (int i = 0; i < NUMBEROFFFITCHAN; i++) {
            _m.Insert( std::string(std::format("type205.ffit_chan[{}].index", i)), std::string(fFringe.t205.ffit_chan[i]->ffit_chan_id));
            _m.Insert( std::string(std::format("type205.ffit_chan[{}].unused ", i)), std::string(fFringe.t205.ffit_chan[i]->unused));
            _m.Insert( std::string("type205.ffit_chan[{}].channels"), create_and_fill_array<short, 4>(fFringe.t205->channels));
        }

        // type_206
        _m.Insert( std::string("type206.record_id"), std::string(fFringe.t206->record_id));
        _m.Insert( std::string("type206.version_no"), std::string(fFringe.t206->version_no));
        _m.Insert( std::string("type206.unused1"), std::string(fFringe.t206->unused1));
        _m.Insert( std::string("type206.start.year"), fFringe.t206.start->year);
        _m.Insert( std::string("type206.start.day"), fFringe.t206.start->day);
        _m.Insert( std::string("type206.start.hour"), fFringe.t206.start->hour);
        _m.Insert( std::string("type206.start.minute"), fFringe.t206.start->minute);
        _m.Insert( std::string("type206.start.second"), fFringe.t206.start->second);
        _m.Insert( std::string("type206.first_ap"), fFringe.t206->first_ap);
        _m.Insert( std::string("type206.last_ap"), fFringe.t206->last_ap);
        for (int i = 0; i < NUMBEROFSIDEBANDSANDSBWEIGHTS; i++) {
            _m.Insert( std::string(std::format("type206.accepted[{}].lsb", i)), fFringe.t206.accepted[i]->lsb);
            _m.Insert( std::string(std::format("type206.accepted[{}].usb", i)), fFringe.t206.accepted[i]->usb);
        }
        for (int i = 0; i < NUMBEROFSIDEBANDSANDSBWEIGHTS; i++) {
            _m.Insert( std::string(std::format("type206.weights[{}].lsb", i)), fFringe.t206.weights[i]->lsb);
            _m.Insert( std::string(std::format("type206.weights[{}].usb", i)), fFringe.t206.weights[i]->usb);
        }
        _m.Insert( std::string("type206.intg_time"), fFringe.t206->intg_time);
        _m.Insert( std::string("type206.accept_ratio"), fFringe.t206->accept_ratio);
        _m.Insert( std::string("type206.discard"), fFringe.t206->discard);
        for (int i = 0; i < NUMBEROFSIDEBANDSANDSBWEIGHTS; i++) {
            _m.Insert( std::string(std::format("type206.reason1[{}].lsb", i)), fFringe.t206.reason1[i]->lsb);
            _m.Insert( std::string(std::format("type206.reason1[{}].usb", i)), fFringe.t206.reason1[i]->usb);
        }
        for (int i = 0; i < NUMBEROFSIDEBANDSANDSBWEIGHTS; i++) {
            _m.Insert( std::string(std::format("type206.reason2[{}].lsb", i)), fFringe.t206.reason2[i]->lsb);
            _m.Insert( std::string(std::format("type206.reason2[{}].usb", i)), fFringe.t206.reason2[i]->usb);
        }
        for (int i = 0; i < NUMBEROFSIDEBANDSANDSBWEIGHTS; i++) {
            _m.Insert( std::string(std::format("type206.reason3[{}].lsb", i)), fFringe.t206.reason3[i]->lsb);
            _m.Insert( std::string(std::format("type206.reason3[{}].usb", i)), fFringe.t206.reason3[i]->usb);
        }
        for (int i = 0; i < NUMBEROFSIDEBANDSANDSBWEIGHTS; i++) {
            _m.Insert( std::string(std::format("type206.reason4[{}].lsb", i)), fFringe.t206.reason4[i]->lsb);
            _m.Insert( std::string(std::format("type206.reason4[{}].usb", i)), fFringe.t206.reason4[i]->usb);
        }
        for (int i = 0; i < NUMBEROFSIDEBANDSANDSBWEIGHTS; i++) {
            _m.Insert( std::string(std::format("type206.reason5[{}].lsb", i)), fFringe.t206.reason5[i]->lsb);
            _m.Insert( std::string(std::format("type206.reason5[{}].usb", i)), fFringe.t206.reason5[i]->usb);
        }
        for (int i = 0; i < NUMBEROFSIDEBANDSANDSBWEIGHTS; i++) {
            _m.Insert( std::string(std::format("type206.reason6[{}].lsb", i)), fFringe.t206.reason6[i]->lsb);
            _m.Insert( std::string(std::format("type206.reason6[{}].usb", i)), fFringe.t206.reason6[i]->usb);
        }
        for (int i = 0; i < NUMBEROFSIDEBANDSANDSBWEIGHTS; i++) {
            _m.Insert( std::string(std::format("type206.reason7[{}].lsb", i)), fFringe.t206.reason7[i]->lsb);
            _m.Insert( std::string(std::format("type206.reason7[{}].usb", i)), fFringe.t206.reason7[i]->usb);
        }
        for (int i = 0; i < NUMBEROFSIDEBANDSANDSBWEIGHTS; i++) {
            _m.Insert( std::string(std::format("type206.reason8[{}].lsb", i)), fFringe.t206.reason8[i]->lsb);
            _m.Insert( std::string(std::format("type206.reason8[{}].usb", i)), fFringe.t206.reason8[i]->usb);
        }
        _m.Insert( std::string("type206.ratesize"), fFringe.t206->ratesize);
        _m.Insert( std::string("type206.mbdsize"), fFringe.t206->mbdsize);
        _m.Insert( std::string("type206.sbdsize"), fFringe.t206->sbdsize);
        _m.Insert( std::string("type206.unused2"), std::string(fFringe.t206->unused2));
    }
}

void MHO_MK4FringeInterface::ExportFringeFileToJSON(){
    // Call typ200 functions here

    // Print out fringe file data
    cout << fFringe;

    //  
}




}
