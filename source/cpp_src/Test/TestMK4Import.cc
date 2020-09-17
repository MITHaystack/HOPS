#include <iostream>
#include <string>
#include <set>
#include <utility>
#include <map>
#include <getopt.h>


#ifdef USE_ROOT
#include "TCanvas.h"
#include "TApplication.h"
#include "TStyle.h"
#include "TColor.h"
#include "TGraph.h"
#include "TGraph2D.h"
#include "TH2D.h"
#include "TMath.h"
#include "TMultiGraph.h"
#endif

#include "HkMK4VexInterface.hh"
#include "HkMK4CorelInterface.hh"

#include "HkVectorContainer.hh"
#include "HkAxisPack.hh"
#include "HkTensorContainer.hh"



using namespace hops;

using visibility_type = std::complex<double>;

using frequency_axis_type = HkVectorContainer<double>;
using time_axis_type = HkVectorContainer<double>;

using channel_axis_pack = HkAxisPack< time_axis_type, frequency_axis_type>;
using channel_data_type = HkTensorContainer< visibility_type, channel_axis_pack >;


double
calc_freq_bin(double ref_sky_freq, double rem_sky_freq, double ref_bw, double rem_bw, char ref_net_sb, char rem_net_sb, int nlags, int bin_index)
{
    double step_sign = 1.0;
    if(ref_net_sb == 'U' && ref_net_sb == 'U')
    {
        step_sign = 1.0;
    }
    else if(ref_net_sb == 'L' && ref_net_sb == 'L')
    {
        step_sign = -1.0;
    }
    else
    {
        std::cout<<"mixed side bands?!"<<std::endl;
    }

    //mixed U/L?

    double freq = 0;
    if( ref_sky_freq == rem_sky_freq && ref_bw == rem_bw)
    {
        freq = ref_sky_freq + bin_index*step_sign*(ref_bw/nlags);
    }
    else
    {
        std::cout<<"ref-rem channel infor mis-match?!"<<std::endl;
    }

    return freq;

}


int main(int argc, char** argv)
{
    std::string usage = "TestMK4Import -r <root_filename> -f <corel_filename>";

    std::string root_filename;
    std::string corel_filename;
    bool have_file = false;

    static struct option longOptions[] = {{"help", no_argument, 0, 'h'},
                                          {"root (vex) file", required_argument, 0, 'r'},
                                          {"corel file", required_argument, 0, 'f'}};

    static const char* optString = "hr:f:";

    while(true)
    {
        char optId = getopt_long(argc, argv, optString, longOptions, NULL);
        if (optId == -1)
            break;
        switch(optId)
        {
            case ('h'):  // help
                std::cout << usage << std::endl;
                return 0;
            case ('r'):
                root_filename = std::string(optarg);
                break;
            case ('f'):
                corel_filename = std::string(optarg);
                break;
            default:
                std::cout << usage << std::endl;
                return 1;
        }
    }

    HkMK4VexInterface mk4vi;
    mk4vi.OpenVexFile(root_filename);
    struct vex* vex_obj = mk4vi.GetVex();

    double ap_time_length = vex_obj->evex->ap_length;

    HkMK4CorelInterface mk4ci;
    mk4ci.ReadCorelFile(corel_filename);
    struct mk4_corel* corel_obj = mk4ci.GetCorel();

    //for the time being we are just going build the visibilities for a single baseline
    std::string baseline = mk4ci.getstr(corel_obj->t100->baseline, 2);

    std::cout<<"test"<<std::endl;
    std::cout<<"ap space"<< corel_obj->index->ap_space<<std::endl;



    //TODO...we need to evaluate whether not this is a feasible approach (probably not)
    //since it is entirely possible for each baseline to have a different number of APs
    //for each channel or to have a different number of lags/spectral points, channels or polarizations
    //How would we store that? A fixed-size tensor may not a be good use of space, should
    //we use/implement a ragged tensor? Or re-organize the data so that
    //'channels' are not physically separate chunks of data, but instead just 'tags'
    //which are associated with chunks of data in a larger array -- this tag mechanism
    //could be used for other things like data flagging, etc.

    //figure out the max number of APs and max lags, as well as the total number
    //of channels and their names
    size_t max_aps = 0;
    size_t max_lags = 0;
    std::set< std::pair<std::string, std::string > > channel_label_pairs;

    struct mk4_corel::index_tag* idx;
    for(int i=0; i<corel_obj->index_space; i++)
    {
        idx = corel_obj->index + i;
        for(int ap=0; ap<idx->ap_space; ap++)
        {
            struct type_120* t120 = idx->t120[ap];
            if(t120 != NULL)
            {
                if(ap > max_aps){max_aps = ap;};
                if(t120->type == SPECTRAL)
                {
                    int nlags = t120->nlags;
                    if(max_lags < nlags){max_lags = nlags;}
                }
            }
        }

        struct type_101* t101;
        if( (t101 = idx->t101) != NULL)
        {
            std::string ref_chan_id = mk4ci.getstr(t101->ref_chan_id,8);
            std::string rem_chan_id = mk4ci.getstr(t101->rem_chan_id,8);
            channel_label_pairs.insert( std::pair<std::string, std::string> (ref_chan_id, rem_chan_id) );
        }
    }

    //create a map of channel-pairs to (empty) channel_data_containers
    size_t dim[2];
    dim[0] = max_aps+1;
    dim[1] = max_lags;
    std::map< std::pair<std::string, std::string >, channel_data_type* > channels;
    for(auto iter = channel_label_pairs.begin(); iter != channel_label_pairs.end(); iter++)
    {
        std::cout<<"channel pair: "<<iter->first<<", "<<iter->second<<std::endl;
        auto key = std::pair<std::string, std::string >(iter->first, iter->second);
        auto chan_it = channels.insert( std::pair< std::pair<std::string, std::string >, channel_data_type* >( key, new channel_data_type(dim) ) );
    }

    //now lets fill them up
    struct type_101* t101 = nullptr;
    for(int i=0; i<corel_obj->index_space; i++)
    {
        idx = corel_obj->index + i;
        if( (t101 = idx->t101) != NULL)
        {
            //extract all of the type101 index records
            std::string ref_chan_id = mk4ci.getstr(t101->ref_chan_id,8);
            std::string rem_chan_id = mk4ci.getstr(t101->rem_chan_id,8);
            auto key = std::pair<std::string, std::string>(ref_chan_id, rem_chan_id);

            auto channel_elem = channels.find(key);
            if( channel_elem != channels.end() )
            {
                //now we want to extract the data in the type_120's
                for(int ap=0; ap<idx->ap_space; ap++)
                {
                    struct type_120* t120 = idx->t120[ap];
                    if(t120 != NULL)
                    {
                        if(t120->type == SPECTRAL)
                        {
                            std::string baseline = mk4ci.getstr(t120->baseline, 2);
                            char ref_st = baseline[0];
                            char rem_st = baseline[1];

                            double ref_sky_freq, ref_bw, rem_sky_freq, rem_bw;
                            char ref_net_sb, rem_net_sb, ref_pol, rem_pol;
                            bool found_ref = false;
                            bool found_rem = false;

                            //now look up the station and channel info so we can fill out the frequncy axis
                            int nst = vex_obj->ovex->nst;
                            for(int ist = 0; ist<nst; ist++)
                            {
                                if(ref_st == vex_obj->ovex->st[ist].mk4_site_id && !found_ref)
                                {
                                    //get the channel information of the reference station
                                    for(size_t nch=0; nch<MAX_CHAN; nch++)
                                    {
                                        std::string chan_name = mk4ci.getstr( vex_obj->ovex->st[ist].channels[nch].chan_name,32);
                                        if(chan_name == key.first)
                                        {
                                            ref_sky_freq = vex_obj->ovex->st[ist].channels[nch].sky_frequency;
                                            ref_bw = vex_obj->ovex->st[ist].channels[nch].bandwidth;
                                            ref_net_sb = vex_obj->ovex->st[ist].channels[nch].net_sideband;
                                            ref_pol = vex_obj->ovex->st[ist].channels[nch].polarization;
                                            found_ref = true;
                                            break;
                                        }
                                    }

                                }

                                if(rem_st == vex_obj->ovex->st[ist].mk4_site_id && !found_rem)
                                {
                                    for(size_t nch=0; nch<MAX_CHAN; nch++)
                                    {
                                        std::string chan_name = mk4ci.getstr( vex_obj->ovex->st[ist].channels[nch].chan_name,32);
                                        if(chan_name == key.second)
                                        {
                                            rem_sky_freq = vex_obj->ovex->st[ist].channels[nch].sky_frequency;
                                            rem_bw = vex_obj->ovex->st[ist].channels[nch].bandwidth;
                                            rem_net_sb = vex_obj->ovex->st[ist].channels[nch].net_sideband;
                                            rem_pol = vex_obj->ovex->st[ist].channels[nch].polarization;
                                            found_rem = true;
                                            break;
                                        }
                                    }

                                }
                                if(found_ref && found_rem){break;}
                            }

                            int nlags = t120->nlags;


                            for(int j=0; j<nlags; j++)
                            {
                                double re = t120->ld.spec[j].re;
                                double im = t120->ld.spec[j].im;
                                channel_elem->second->at(ap,j) = std::complex<double>(re,im);

                                //set up the frequency axis
                                double freq =  calc_freq_bin(ref_sky_freq, rem_sky_freq, ref_bw, rem_bw, ref_net_sb,rem_net_sb, nlags, j);
                                std::get<1>(*(channel_elem->second))(j) = freq;
                            }
                        }
                        else
                        {
                            std::cout<<"non-spectral type-120 not supported."<<std::endl;
                        }
                    }
                    //now assign values to the time (0-th dim) axis
                    std::get<0>(*(channel_elem->second))(ap) = ap_time_length*ap;
                }
            }
        }

    }//end of index loop

    #ifdef USE_ROOT


    std::cout<<"starting root plotting"<<std::endl;

    //ROOT stuff for plots
    TApplication* App = new TApplication("PowerPlot",&argc,argv);
    TStyle* myStyle = new TStyle("Plain", "Plain");
    myStyle->SetCanvasBorderMode(0);
    myStyle->SetPadBorderMode(0);
    myStyle->SetPadColor(0);
    myStyle->SetCanvasColor(0);
    myStyle->SetTitleColor(1);
    myStyle->SetPalette(1,0);   // nice color scale for z-axis
    myStyle->SetCanvasBorderMode(0); // gets rid of the stupid raised edge around the canvas
    myStyle->SetTitleFillColor(0); //turns the default dove-grey background to white
    myStyle->SetCanvasColor(0);
    myStyle->SetPadColor(0);
    myStyle->SetTitleFillColor(0);
    myStyle->SetStatColor(0); //this one may not work
    const int NRGBs = 5;
    const int NCont = 48;
    double stops[NRGBs] = { 0.00, 0.34, 0.61, 0.84, 1.00 };
    double red[NRGBs]   = { 0.00, 0.00, 0.87, 1.00, 0.51 };
    double green[NRGBs] = { 0.00, 0.81, 1.00, 0.20, 0.00 };
    double blue[NRGBs]  = { 0.51, 1.00, 0.12, 0.00, 0.00 };
    TColor::CreateGradientColorTable(NRGBs, stops, red, green, blue, NCont);
    myStyle->SetNumberContours(NCont);
    myStyle->cd();


    auto first_channel_iter = channels.begin();
    auto first_channel = first_channel_iter->second;
    auto* x_axis = &(std::get<0>(*first_channel));
    auto* y_axis = &(std::get<1>(*first_channel));

    size_t x_axis_size = x_axis->GetSize();
    size_t y_axis_size = y_axis->GetSize();

    //just plot phases for a single channel
    TGraph2D *gr = new TGraph2D(x_axis_size*y_axis_size);
    TGraph2D *gb = new TGraph2D(x_axis_size*y_axis_size);

    size_t count = 0;
    for(size_t i=0; i<x_axis_size; i++)
    {
        for(size_t j=0; j<y_axis_size; j++)
        {
            std::complex<double> vis = first_channel->at(i,j);
            gr->SetPoint(count, x_axis->at(i), y_axis->at(j), std::arg(vis) );
            gb->SetPoint(count, x_axis->at(i), y_axis->at(j), std::abs(vis) );
            //std::cout<<"i,j = "<<i<<", "<<j<<std::endl;
            count++;
        }
    }

    std::string name = first_channel_iter->first.first + "-" +  first_channel_iter->first.second;
    TCanvas* c = new TCanvas(name.c_str(),name.c_str(), 50, 50, 950, 850);
    c->SetFillColor(0);
    c->SetRightMargin(0.2);
    c->Divide(1,2);
    c->cd(1);
    gr->Draw("COLZ");
    c->Update();
    c->cd(2);
    gb->Draw("COLZ");
    c->Update();

    App->Run();

    #endif




    return 0;
}
