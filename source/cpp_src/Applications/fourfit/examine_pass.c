#include "hops_complex.h"
#include "mk4_data.h"
#include "pass_struct.h"


#include <algorithm>

//global messaging util
#include "MHO_Message.hh"

//global messaging util
#include "MHO_Message.hh"

//handles reading directories, listing files etc.
#include "MHO_DirectoryInterface.hh"

//needed to read hops files and extract objects
#include "MHO_ContainerDefinitions.hh"
#include "MHO_ContainerStore.hh"
#include "MHO_ContainerDictionary.hh"
#include "MHO_ContainerFileInterface.hh"

using namespace hops;

void examine_pass(struct type_pass* pass, int pass_index)
{
    msg_info("nufourfit", "dumping vis data from pass: " << pass_index << eom);

    int npolprod = 4;
    int nchan = 0;
    int nap = 0;
    int nlags = 0;

    int sb = 1;// ONLY LSB RIGHT NOW

    nchan = pass->nfreq;
    nap = pass->num_ap;
    int pol = pass->pol;

    for(int ch=0; ch < nchan; ch++)
    {
        double chan_freq = pass->pass_data[ch].frequency;
        std::cout<<"chan: "<<ch<<" freq = "<<chan_freq<<std::endl;
        std::cout<<"polprod code = "<<pass->pol<<std::endl;
        if(pol == 0)
        {
            int sband = pass->pass_data[ch].data->sband;

            //for(int sb=0; sb<2; sb++)
            {
                if(pass->pass_data[ch].data->apdata_ll[sb] != NULL)
                {
                    std::cout<<"sband = "<<sb<<std::endl;
                    std::cout<<"nlags = "<<pass->pass_data[ch].data->apdata_ll[sb]->nlags <<std::endl;
                    nlags = std::max(nlags, (int) pass->pass_data[ch].data->apdata_ll[sb]->nlags);
                }
            }
        }
    }


    //lets extract the pass data into a visibility container
    visibility_type* bl_data = new visibility_type();
    weight_type* wt_data = new weight_type();
    bl_data->Resize(npolprod, nchan, nap, nlags);
    bl_data->ZeroArray();
    wt_data->Resize(npolprod, nchan, nap, 1);
    wt_data->ZeroArray();


    for(int pp=0; pp<npolprod; pp++)
    {
        std::string pp_label = "";
        //pass->linpol; ///[2]; TODO use lin-pol indicators to get the correct pol prod label
        if(pp == POL_LL){pp_label = "LL";}
        if(pp == POL_RR){pp_label = "RR";}
        if(pp == POL_LR){pp_label = "LR";}
        if(pp == POL_RL){pp_label = "RL";}

        std::get<POLPROD_AXIS>(*bl_data)(pp) = pp_label;
        std::get<POLPROD_AXIS>(*wt_data)(pp) = pp_label;

        for(int ch=0; ch<nchan; ch++)
        {
            //set sky freq on channel axis
            double chan_freq = pass->pass_data[ch].frequency;
            std::get<CHANNEL_AXIS>(*bl_data)(ch) = chan_freq;
            std::get<CHANNEL_AXIS>(*wt_data)(ch) = chan_freq;
            for(int ap=0; ap<nap; ap++)
            {
                std::get<TIME_AXIS>(*bl_data)(ap) = ap; //not correct, should be scaled by ap_interval
                std::get<TIME_AXIS>(*wt_data)(ap) = ap;
                for(int n=0; n<nlags; n++)
                {
                    std::get<FREQ_AXIS>(*bl_data)(n) = n; //not correct, should be scaled by freq interval
                    auto lag_ptr = pass->pass_data[ch].data[0].apdata_ll[sb];
                    lag_ptr = nullptr;
                    if(pp == POL_LL){lag_ptr = pass->pass_data[ch].data[ap].apdata_ll[sb];}
                    if(pp == POL_RR){lag_ptr = pass->pass_data[ch].data[ap].apdata_rr[sb];}
                    if(pp == POL_LR){lag_ptr = pass->pass_data[ch].data[ap].apdata_lr[sb];}
                    if(pp == POL_RL){lag_ptr = pass->pass_data[ch].data[ap].apdata_rl[sb];}

                    if( lag_ptr != NULL)
                    {
                        double rcomp = lag_ptr->ld.spec[n].re;
                        double icomp = lag_ptr->ld.spec[n].im;
                        std::complex<double> vis(rcomp, icomp);
                        (*bl_data)(pp,ch,ap,n) = vis;

                        if(n==0)
                        {
                            double w = lag_ptr->fw.weight;
                            (*wt_data)(pp,ch,ap,0) = w;
                        }
                    }
                }
            }
        }
    }

    //dump bl_data into a file for later inspection
    std::stringstream ss;
    ss << "./pass_";
    ss << pass_index;
    ss << ".dump";

    std::string output_file = ss.str();
    MHO_BinaryFileInterface inter;
    bool status = inter.OpenToWrite(output_file);
    if(status)
    {
        uint32_t label = 0xFFFFFFFF; //someday make this mean something
        inter.Write(*bl_data, "vis", label);
        inter.Write(*wt_data, "weight", label);
    }
    else
    {
        msg_error("file", "Error opening corel output file: " << output_file << eom);
    }

    inter.Close();


    delete bl_data;
    delete wt_data;

}































void examine_pass_sbd(struct type_pass* pass, int pass_index)
{
    msg_info("nufourfit", "dumping sbd data from pass: " << pass_index << eom);

    int npolprod = 4;
    int nchan = 0;
    int nap = 0;
    int nlags = 0;

    int sb = 1;// ONLY LSB RIGHT NOW

    nchan = pass->nfreq;
    nap = pass->num_ap;
    int pol = pass->pol;

    for(int ch=0; ch < nchan; ch++)
    {
        double chan_freq = pass->pass_data[ch].frequency;
        std::cout<<"chan: "<<ch<<" freq = "<<chan_freq<<std::endl;
        std::cout<<"polprod code = "<<pass->pol<<std::endl;
        if(pol == 0)
        {
            int sband = pass->pass_data[ch].data->sband;

            //for(int sb=0; sb<2; sb++)
            {
                if(pass->pass_data[ch].data->apdata_ll[sb] != NULL)
                {
                    std::cout<<"sband = "<<sb<<std::endl;
                    std::cout<<"nlags = "<<pass->pass_data[ch].data->apdata_ll[sb]->nlags <<std::endl;
                    nlags = std::max(nlags, (int) pass->pass_data[ch].data->apdata_ll[sb]->nlags);
                }
            }
        }
    }

    //lets extract the pass data into a visibility container
    visibility_type* sbd_data = new visibility_type();

    sbd_data->Resize(npolprod, nchan, nap, nlags);
    sbd_data->ZeroArray();

    for(int pp=0; pp<npolprod; pp++)
    {
        std::string pp_label = "";
        //pass->linpol; ///[2]; TODO use lin-pol indicators to get the correct pol prod label
        if(pp == POL_LL){pp_label = "LL";}
        if(pp == POL_RR){pp_label = "RR";}
        if(pp == POL_LR){pp_label = "LR";}
        if(pp == POL_RL){pp_label = "RL";}

        std::get<POLPROD_AXIS>(*sbd_data)(pp) = pp_label;
        for(int ch=0; ch<nchan; ch++)
        {
            //set sky freq on channel axis
            double chan_freq = pass->pass_data[ch].frequency;
            std::get<CHANNEL_AXIS>(*sbd_data)(ch) = chan_freq;
            for(int ap=0; ap<nap; ap++)
            {
                std::get<TIME_AXIS>(*sbd_data)(ap) = ap; //not correct, should be scaled by ap_interval
                auto sbd_ptr = pass->pass_data[ch].data[ap].sbdelay;
                for(int n=0; n<nlags; n++) //TODO check the size of sbdelay (should be 2*nlags ?)
                {
                    std::get<FREQ_AXIS>(*sbd_data)(n) = n; //not correct, should be scaled by freq interval

                    if( sbd_ptr != NULL)
                    {

                        double rcomp = real_comp(sbd_ptr[n]);
                        double icomp = imag_comp(sbd_ptr[n]);
                        std::complex<double> sbd(rcomp, icomp);
                        (*sbd_data)(pp,ch,ap,n) = sbd;
                    }
                }
            }
        }
    }

    //dump bl_data into a file for later inspection
    std::stringstream ss;
    ss << "./pass_sbd_";
    ss << pass_index;
    ss << ".dump";

    std::string output_file = ss.str();
    MHO_BinaryFileInterface inter;
    bool status = inter.OpenToWrite(output_file);
    if(status)
    {
        uint32_t label = 0xFFFFFFFF; //someday make this mean something
        inter.Write(*sbd_data, "sbd", label);
    }
    else
    {
        msg_error("file", "Error opening corel output file: " << output_file << eom);
    }

    inter.Close();
    delete sbd_data;

}
