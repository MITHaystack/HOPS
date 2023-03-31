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

void examine_pass(struct type_pass* pass)
{
    printf("EXAMINING PASS_STRUCT, nfreq = %d \n", pass->nfreq);

    msg_info("nufourfit", "test message. " <<eom);

    int npolprod = 1;//only examinining 1 pol prod right now
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
        for(int ch=0; ch<nchan; ch++)
        {
            for(int ap=0; ap<nap; ap++)
            {
                for(int n=0; n<nlags; n++)
                {
                    if(pass->pass_data[ch].data->apdata_ll[sb] != NULL)
                    {

                        double rcomp = pass->pass_data[ch].data[ap].apdata_ll[sb]->ld.spec[n].re;
                        double icomp = pass->pass_data[ch].data[ap].apdata_ll[sb]->ld.spec[n].im;
                        std::complex<double> vis(rcomp, icomp);
                        (*bl_data)(pp,ch,ap,n) = vis;
                    }
                }
            }
        }
    }

    std::cout<<"blah = "<<*bl_data<<std::endl;

    delete bl_data;

}
