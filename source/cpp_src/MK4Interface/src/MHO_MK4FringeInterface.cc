#include "MHO_MK4FringeInterface.hh"
#include "MHO_MK4Type203Converter.hh"
#include "MHO_MK4Type205Converter.hh"

#include "MHO_MultiTypeMap.hh"
#include <array>
#include <iostream>
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
MHO_MK4FringeInterface::ExportFringeFilesToStructs()
{
    if(fHaveFringe)
    {
        //want to dump the information in the type_200 through type_230 objects
        //for now just do the POD data types

        MHO_MultiTypeMap< std::string, int, short, float, double, std::array<double, 4>, std::string> _m;

        // convert type_200 data to struct
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

        // convert type_201 data to struct
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

        // convert type_202 data to struct
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

        // convert type_203 data to struct
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

        // convert type_204 data to struct
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

        // convert type_205 data to struct
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

        // convert type_206 data to struct
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

        // convert type_207 data to struct
        _m.Insert( std::string("type207.record_id"), std::string(fFringe.t207->record_id));
        _m.Insert( std::string("type207.version_no"), std::string(fFringe.t207->version_no));
        _m.Insert( std::string("type207.unused1"), std::string(fFringe.t207->unused1));
        _m.Insert( std::string("type207.pcal_mode"), fFringe.t207->pcal_mode);
        _m.Insert( std::string("type207.unused2"), fFringe.t207->unused2);
        for (int i = 0; i < REFANDREMSIZE; i++) {
            _m.Insert( std::string(std::format("type207.ref_pcamp[{}].lsb", i)), fFringe.t207.ref_pcamp[i]->lsb);
            _m.Insert( std::string(std::format("type207.ref_pcamp[{}].usb", i)), fFringe.t207.ref_pcamp[i]->usb);
        }
        for (int i = 0; i < REFANDREMSIZE; i++) {
            _m.Insert( std::string(std::format("type207.rem_pcamp[{}].lsb", i)), fFringe.t207.rem_pcamp[i]->lsb);
            _m.Insert( std::string(std::format("type207.rem_pcamp[{}].usb", i)), fFringe.t207.rem_pcamp[i]->usb);
        }
        for (int i = 0; i < REFANDREMSIZE; i++) {
            _m.Insert( std::string(std::format("type207.ref_pcphase[{}].lsb", i)), fFringe.t207.ref_pcphase[i]->lsb);
            _m.Insert( std::string(std::format("type207.ref_pcphase[{}].usb", i)), fFringe.t207.ref_pcphase[i]->usb);
        }
        for (int i = 0; i < REFANDREMSIZE; i++) {
            _m.Insert( std::string(std::format("type207.rem_pcphase[{}].lsb", i)), fFringe.t207.rem_pcphase[i]->lsb);
            _m.Insert( std::string(std::format("type207.rem_pcphase[{}].usb", i)), fFringe.t207.rem_pcphase[i]->usb);
        }
        for (int i = 0; i < REFANDREMSIZE; i++) {
            _m.Insert( std::string(std::format("type207.ref_pcoffset[{}].lsb", i)), fFringe.t207.ref_pcoffset[i]->lsb);
            _m.Insert( std::string(std::format("type207.ref_pcoffset[{}].usb", i)), fFringe.t207.ref_pcoffset[i]->usb);
        }
        for (int i = 0; i < REFANDREMSIZE; i++) {
            _m.Insert( std::string(std::format("type207.rem_pcoffset[{}].lsb", i)), fFringe.t207.rem_pcoffset[i]->lsb);
            _m.Insert( std::string(std::format("type207.rem_pcoffset[{}].usb", i)), fFringe.t207.rem_pcoffset[i]->usb);
        }
        for (int i = 0; i < REFANDREMSIZE; i++) {
            _m.Insert( std::string(std::format("type207.rem_pcfreq[{}].lsb", i)), fFringe.t207.rem_pcfreq[i]->lsb);
            _m.Insert( std::string(std::format("type207.rem_pcfreq[{}].usb", i)), fFringe.t207.rem_pcfreq[i]->usb);
        }
        _m.Insert( std::string("type207.rem_pcrate"), fFringe.t207->rem_pcrate);
        _m.Insert( std::string("type207.ref_pcrate"), fFringe.t207->ref_pcrate);
        for (int i = 0; i < REFANDREMSIZE; i++) {
            _m.Insert( std::string(std::format("type207.ref_errate[{}]", i)), fFringe.t207->ref_errate[i]);
        }
        for (int i = 0; i < REFANDREMSIZE; i++) {
            _m.Insert( std::string(std::format("type207.rem_errate[{}]", i)), fFringe.t207->rem_errate[i]);
        }

        // convert type_208 data to struct
        _m.Insert( std::string("type208.record_id"), std::string(fFringe.t208->record_id));
        _m.Insert( std::string("type208.version_no"), std::string(fFringe.t208->version_no));
        _m.Insert( std::string("type208.unused1"), std::string(fFringe.t208->unused1));
        _m.Insert( std::string("type208.quality"), std::string(fFringe.t208->quality));
        _m.Insert( std::string("type208.errcode"), std::string(fFringe.t208->errcode));
        _m.Insert( std::string("type208.tape_qcode"), std::string(fFringe.t208->tape_qcode));
        _m.Insert( std::string("type208.adelay"), fFringe.t208->adelay);
        _m.Insert( std::string("type208.arate"), fFringe.t208->arate);
        _m.Insert( std::string("type208.aaccel"), fFringe.t208->aaccel);
        _m.Insert( std::string("type208.tot_mbd"), fFringe.t208->tot_mbd);
        _m.Insert( std::string("type208.tot_sbd"), fFringe.t208->tot_sbd);
        _m.Insert( std::string("type208.tot_rate"), fFringe.t208->tot_rate);
        _m.Insert( std::string("type208.tot_mbd_ref"), fFringe.t208->tot_mbd_ref);
        _m.Insert( std::string("type208.resid_mbd"), fFringe.t208->resid_mbd);
        _m.Insert( std::string("type208.resid_sbd"), fFringe.t208->resid_sbd);
        _m.Insert( std::string("type208.resid_rate"), fFringe.t208->resid_rate);
        _m.Insert( std::string("type208.mbd_error "), fFringe.t208->mbd_error);
        _m.Insert( std::string("type208.sbd_error"), fFringe.t208->sbd_error);
        _m.Insert( std::string("type208.rate_error"), fFringe.t208->rate_error);
        _m.Insert( std::string("type208.ambiguity"), fFringe.t208->ambiguity);
        _m.Insert( std::string("type208.amplitude"), fFringe.t208->amplitude);
        _m.Insert( std::string("type208.inc_seg_ampl"), fFringe.t208->inc_seg_ampl);
        _m.Insert( std::string("type208.inc_chan_ampl"), fFringe.t208->inc_chan_ampl);
        _m.Insert( std::string("type208.snr"), fFringe.t208->snr);
        _m.Insert( std::string("type208.prob_false"), fFringe.t208->prob_false);
        _m.Insert( std::string("type208.totphase"), fFringe.t208->totphase);
        _m.Insert( std::string("type208.totphase_ref"), fFringe.t208->totphase_ref);
        _m.Insert( std::string("type208.resphase"), fFringe.t208->resphase);
        _m.Insert( std::string("type208.tec_error"), fFringe.t208->tec_error);

        // convert type_210 data to struct
        _m.Insert( std::string("type210.record_id"), std::string(fFringe.t210->record_id));
        _m.Insert( std::string("type210.version_no"), std::string(fFringe.t210->version_no));
        _m.Insert( std::string("type210.unused1"), std::string(fFringe.t210->unused1));
        for (int i = 0; i < AMPPHASE; i++) {
            _m.Insert( std::string(std::format("type210.amp_phas[{}].ampl", i)), fFringe.t210.amp_phas[i]->ampl);
            _m.Insert( std::string(std::format("type210.amp_phas[{}].phase", i)), fFringe.t210.amp_phas[i]->phase);
        }

        // convert type_212 data to struct
        _m.Insert( std::string("type212.record_id"), std::string(fFringe.t212->record_id));
        _m.Insert( std::string("type212.version_no"), std::string(fFringe.t212->version_no));
        _m.Insert( std::string("type212.unused"), std::string(fFringe.t212->unused));
        _m.Insert( std::string("type212.nap"), fFringe.t212->nap);
        _m.Insert( std::string("type212.first_ap"), fFringe.t212->first_ap);
        _m.Insert( std::string("type212.channel"), fFringe.t212->channel);
        _m.Insert( std::string("type212.sbd_chan"), fFringe.t212->sbd_chan);
        _m.Insert( std::string("type212.unused2"), std::string(fFringe.t212->unused2));
        for (int i = 0; i < DATASIZE; i++) {
            _m.Insert( std::string(std::format("type212.data[{}].amp", i)), fFringe.t212.data[i]->amp);
            _m.Insert( std::string(std::format("type212.data[{}].phase", i)), fFringe.t212.data[i]->phase);
            _m.Insert( std::string(std::format("type212.data[{}].weight", i)), fFringe.t212.data[i]->weight);
        }
    }
}

void MHO_MK4FringeInterface::ExportFringeStructsToJSON(){
    // Call typ200 functions here
    json jsonDump = {{"type_200", MHO_MK4Type200Converter::convertToJSON(fFringe->t200)},
                    {"type_201", MHO_MK4Type201Converter::convertToJSON(fFringe->t201)},
                    {"type_202", MHO_MK4Type202Converter::convertToJSON(fFringe->t202)},
                    {"type_203", MHO_MK4Type203Converter::convertToJSON(fFringe->t203)},
                    {"type_204", MHO_MK4Type204Converter::convertToJSON(fFringe->t204)},
                    {"type_205", MHO_MK4Type205Converter::convertToJSON(fFringe->t205)},
                    {"type_206", MHO_MK4Type206Converter::convertToJSON(fFringe->t206)},
                    {"type_207", MHO_MK4Type207Converter::convertToJSON(fFringe->t207)},
                    {"type_208", MHO_MK4Type208Converter::convertToJSON(fFringe->t208)},
                    {"type_210", MHO_MK4Type210Converter::convertToJSON(fFringe->t210)},
                    {"type_212", MHO_MK4Type212Converter::convertToJSON(fFringe->t212)},
    }

    // Print out fringe file data
    //cout << fFringe;
    std::ofstream o("type-200s-dump.json");
    o << std::setw(4) << jsonDump << std::endl;

    //  
}

void MHO_MK4FringeInterface::ExportFringeFilesToJSON(const std::string& inputFile){
    // call ReadFringeFile
    // call ExportFringeFilesToStructs
    // call ExportFringeStructsToJSON
    std::cout << inputFile << endl;
    // if (ReadFringeFile(inputFile)) {
        // self.ExportFringeFilesToStructs();
        // self.ExportFringeStructsToJSON();

    //}

}

}
