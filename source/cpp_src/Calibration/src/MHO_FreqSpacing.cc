#include "MHO_FreqSpacing.hh"
#include <iostream>

namespace hops
{

void FreqSpacing(const channel_axis_type& ch_axis)
{

    int fr, div, grid_pts, spacing_ok;

    std::size_t n_freq, n_chan;
    n_chan = ch_axis.GetSize();
    double freq, min_freq, min_space, ave_freq, spacing, index, space;
    double epsilon = 0.001;

    //grab all the channel sky frequencies
    std::string sky_freq_key = "sky_freq";
    std::vector< double > chan_freqs;
    for(std::size_t i=0;i<n_chan; i++)
    {
        std::vector< const MHO_IntervalLabel* > labels;
        labels = ch_axis.GetIntervalsWhichIntersect(i);
        if(labels.size() != 0)
        {
            for(std::size_t j=0; j < labels.size(); j++)
            {
                if(labels[j]->HasKey(sky_freq_key))
                {
                    labels[j]->Retrieve(sky_freq_key, freq);
                    chan_freqs.push_back(freq);
                }
                else{std::cout<<"no sky_freq"<<std::endl;}
            }
        }
        else{std::cout<<"no labels for chan: "<<i<<std::endl;}
    }



    //find the min freq, the average freq, spacing, etc.
    n_freq = chan_freqs.size();

    if(n_freq == 0){std::cout<<"Error"<<std::endl;}

    min_freq = std::numeric_limits<double>::max();
    min_space = std::numeric_limits<double>::max();
    ave_freq = 0.0;
    for(std::size_t i=0; i<n_freq; i++)
    {
        double freq = chan_freqs[i];
        if(freq < min_freq){min_freq = freq;}
        ave_freq += freq;
        for(std::size_t j=i+1; j<n_freq; j++)
        {
            space = std::fabs(chan_freqs[j] - freq );
            std::cout<<"delta("<<i<<", "<<j<<") = "<<space<<std::endl;
            //TODO FIXME floating point comparison with zero can be unreliable
            if(space < min_space && space != 0){min_space = space;}
        }
    }
    ave_freq /= (double)n_freq;

    std::cout<<"min freq = "<<min_freq<<" min_space = "<<min_space<<" ave_freq = "<<ave_freq<<std::endl;

    //msg ("min_freq %lf min_space %lf ave_freq %lf", -1, min_freq, min_space, ave_freq);
    div = 1;

    bool spacing_err = false;
    std::map<std::size_t, double> chan_idx_to_mbd_idx;

    do                                  /* Yup, its one of them thar do-while loops */
    {
        spacing_ok = 1;
        spacing = min_space / div;      /* Sub-divide the spacing  */        
        div++;
        grid_pts = 2;
        for(std::size_t fr = 0; fr < n_freq; fr++)
        {
            index = (chan_freqs[fr] - min_freq) / spacing;
                                        /* Check whether all freqs */
                                        /* lie on grid points */
            if (fabs(index - (int)(index+0.5)) > epsilon){spacing_ok = 0;}

            index = (double) (int)(index + 0.5);
                                        /* Make # of grid points the smallest  */
            for(int i = 1; i < 8; i++)     /* power of 2 that will cover all points */
            {
                if( (grid_pts - 1) < index)
                {
                    grid_pts *= 2;
                }
            }

            if ((index > (GRID_PTS-1)) || (index < 0))
            {
                spacing_err = true;
                chan_idx_to_mbd_idx[fr] = BOGUS_MB_INDEX;
                // status.space_err = 1;
                // status.mb_index[fr] = BOGUS_MB_INDEX;
            }
            else
            {
                chan_idx_to_mbd_idx[fr] = index;
                //status.mb_index[fr] = index;
            }
        }
    }
    while ((div < 256) && (spacing_ok == 0));


    for(auto it = chan_idx_to_mbd_idx.begin(); it != chan_idx_to_mbd_idx.end(); it++)
    {
        std::cout<<"chan2mbd idx = "<<it->first<<", "<<it->second<<std::endl;
    }


    // if (status.space_err==1)
    //     msg ("Frequency spacing too large for FFT, check f sequence or array dims!!", 2);
    // status.freq_space = spacing;
    // status.grid_points = grid_pts; 
    // msg ("spacing %g grid_pts %d", 0, spacing, grid_pts);
    // 
    // if(status.grid_points > GRID_PTS)
    //     status.grid_points = GRID_PTS;
    // status.grid_points *= MBDMULT;
    //                                     /* This needs to account for mixed */
    //                                     /* single and double sideband data */
    //                                     /* Should it be accurately weighted? */
    //                                     /* Seems best diagnostic of sideband */
    //                                     /* presence is ap_num.  Perhaps proper */
    //                                     /* treatment is to invent then weight */
    //                                     /* ap_num_frac.  Discuss on return */
    // // order of code is reversed here -- if pass->nfreq == 1 the loop is not needed...
    // // freq_spread is used in fill_208 to estimate the snr of the mbd value
    // status.freq_spread = 0.0;
    // for (fr = 0; fr < pass->nfreq; fr++)
    //     {
    //     status.freq_spread += (pass->pass_data[fr].frequency - ave_freq) 
    //                             * (pass->pass_data[fr].frequency - ave_freq);
    //     msg ("freq[%d] %f mb_index %d", 0, fr, pass->pass_data[fr].frequency, status.mb_index[fr]);
    //     }
    // if (pass->nfreq > 1) 
    //     status.freq_spread= sqrt(status.freq_spread / pass->nfreq);
    // else if (pass->nfreq == 1) 
    //     status.freq_spread = 1.0 / (param.samp_period * 2.0E6 * sqrt(12.0));
    // msg ("grid_points %d freq_spread %lf", 0, status.grid_points, status.freq_spread);
}


}
