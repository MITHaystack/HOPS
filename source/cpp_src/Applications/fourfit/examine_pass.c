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
    printf("EXAMINING PASS_STRUCT, nfreq = %d \n", pass->nfreq);

    msg_info("nufourfit", "test message. " <<eom);

    int npolprod = 4;//only examinining 1 pol prod right now
    //
    // for(int p=0; p<4; p++)
    // {
    //     if(pass->pprods_present[p]){npolprod++;}
    // }

    int nchan = 0;
    int nap = 0;
    int nlags = 0;

    int sb = 1;// ONLY LBS RIGHT NOW

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
    bl_data->Resize(npolprod, nchan, nap, nlags);
    bl_data->ZeroArray();

    for(int pp=0; pp<npolprod; pp++)
    {
        std::string pp_label = "";
        //pass->linpol; ///[2]; TODO use lin-pol indicators to get the correct pol prod label
        if(pp == POL_LL){pp_label = "LL";}
        if(pp == POL_RR){pp_label = "RR";}
        if(pp == POL_LR){pp_label = "LR";}
        if(pp == POL_RL){pp_label = "RL";}

        std::get<POLPROD_AXIS>(*bl_data)(pp) = pp_label;// placeholder
        for(int ch=0; ch<nchan; ch++)
        {
            //set sky freq on channel axis
            double chan_freq = pass->pass_data[ch].frequency;
            std::get<CHANNEL_AXIS>(*bl_data)(ch) = chan_freq;
            for(int ap=0; ap<nap; ap++)
            {
                std::get<TIME_AXIS>(*bl_data)(ap) = ap; //not correct, should be scaled by ap_interval
                for(int n=0; n<nlags; n++)
                {
                    std::get<FREQ_AXIS>(*bl_data)(n) = n; //not correct, should be scaled by freq interval
                    auto lag_ptr = pass->pass_data[ch].data->apdata_ll[sb];
                    lag_ptr = nullptr;
                    if(pp == POL_LL){lag_ptr = pass->pass_data[ch].data->apdata_ll[sb];}
                    if(pp == POL_RR){lag_ptr = pass->pass_data[ch].data->apdata_rr[sb];}
                    if(pp == POL_LR){lag_ptr = pass->pass_data[ch].data->apdata_lr[sb];}
                    if(pp == POL_RL){lag_ptr = pass->pass_data[ch].data->apdata_rl[sb];}
                    if( lag_ptr != NULL)
                    {
                        double rcomp = lag_ptr->ld.spec[n].re;
                        double icomp = lag_ptr->ld.spec[n].im;
                        std::complex<double> vis(rcomp, icomp);
                        (*bl_data)(pp,ch,ap,n) = vis;
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
        //inter.Write(*ch_bl_wdata, "weight", label);
    }
    else
    {
        msg_error("file", "Error opening corel output file: " << output_file << eom);
    }

    inter.Close();


    delete bl_data;

}
