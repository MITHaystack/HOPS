#include "MHO_UniformGridPointsCalculator.hh"
#include "MHO_Message.hh"


namespace hops 
{

MHO_UniformGridPointsCalculator::MHO_UniformGridPointsCalculator()
{
    fEpsilon = 0.001;
    fSpacing = 0;
    fMinSpacing = 0;
    fMaxSpacing = 0;
    fNGridPoints = 0;
    fAbsEps = 1e-30;
}

MHO_UniformGridPointsCalculator::~MHO_UniformGridPointsCalculator(){};


void 
MHO_UniformGridPointsCalculator::SetPoints(std::vector<double> pts)
{
    fPoints.clear();
    fPoints = pts;
}

void 
MHO_UniformGridPointsCalculator::Calculate()
{
    Calculate_v2();
}

void 
MHO_UniformGridPointsCalculator::Calculate_v1()
{
    std::size_t MBD_MAXPTS = 8192;
    std::size_t MBD_MULT = 4;
    std::size_t MBD_GRID_PTS = MBD_MAXPTS / MBD_MULT;
    std::size_t BOGUS_MBD_INDEX = MBD_GRID_PTS*MBD_MULT + 1;

    fIndexMap.clear();
    if(fPoints.size() != 0)
    {
        int fr, div, grid_pts, spacing_ok;

        std::size_t n_pts;
        double freq, min_pts, min_space, ave_loc, spacing, index, space;

        //find the min freq, the average freq, spacing, etc.
        n_pts = fPoints.size();

        if(n_pts == 0){std::cout<<"Error"<<std::endl;}

        min_pts = std::numeric_limits<double>::max();
        min_space = std::numeric_limits<double>::max();
        ave_loc = 0.0;
        for(std::size_t i=0; i<n_pts; i++)
        {
            double freq = fPoints[i];
            if(freq < min_pts){min_pts = freq;}
            ave_loc += freq;
            for(std::size_t j=i+1; j<n_pts; j++)
            {
                space = std::fabs(fPoints[j] - freq );
                std::cout<<"delta("<<i<<", "<<j<<") = "<<space<<std::endl;
                //TODO FIXME floating point comparison with zero can be unreliable
                if(space < min_space && space != 0){min_space = space;}
            }
        }
        ave_loc /= (double)n_pts;

        std::cout<<"min freq = "<<min_pts<<" min_space = "<<min_space<<" ave_loc = "<<ave_loc<<std::endl;

        //msg ("min_pts %lf min_space %lf ave_loc %lf", -1, min_pts, min_space, ave_loc);
        div = 1;

        bool spacing_err = false;
        std::map<std::size_t, double> chan_idx_to_mbd_idx;

        do                                  /* Yup, its one of them thar do-while loops */
        {
            spacing_ok = 1;
            spacing = min_space / div;      /* Sub-divide the spacing  */        
            div++;
            grid_pts = 2;
            for(std::size_t fr = 0; fr < n_pts; fr++)
            {
                index = (fPoints[fr] - min_pts) / spacing;
                                            /* Check whether all freqs */
                                            /* lie on grid points */
                if (fabs(index - (int)(index+0.5)) > fEpsilon){spacing_ok = 0;}

                index = (double) (int)(index + 0.5);
                                            /* Make # of grid points the smallest  */
                for(int i = 1; i < 8; i++)     /* power of 2 that will cover all points */
                {
                    if( (grid_pts - 1) < index)
                    {
                        grid_pts *= 2;
                    }
                }

                if ((index > (MBD_GRID_PTS-1)) || (index < 0))
                {
                    spacing_err = true;
                    chan_idx_to_mbd_idx[fr] = BOGUS_MBD_INDEX;
                    fIndexMap[fr] = (std::size_t) index;
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


        std::cout<<"N MBD grid pts = "<<grid_pts<<std::endl;

        for(auto it = chan_idx_to_mbd_idx.begin(); it != chan_idx_to_mbd_idx.end(); it++)
        {
            std::cout<<"chan2mbd idx = "<<it->first<<", "<<it->second<<std::endl;
        }

        fSpacing = min_space;
        fStart = min_pts;
        fNGridPoints = grid_pts;
    }
    else 
    {
        msg_error("math", "cannot construct uniform grid with no points given." << eom);
    }

}




void 
MHO_UniformGridPointsCalculator::Calculate_v2()
{
    fIndexMap.clear();
    if(fPoints.size() != 0)
    {

        fStart = std::numeric_limits<double>::max();
        fMinSpacing = std::numeric_limits<double>::max();
        fMaxSpacing = 0;

        //look for min point and min spacing between any pair of points
        std::size_t npts = fPoints.size();
        for(std::size_t i=0; i<npts; i++)
        {
            double val = fPoints[i];
            if(val < fStart){fStart = val;}
            for(std::size_t j=i+1; j<npts; j++)
            {
                double space = std::fabs(fPoints[j] - val);
                std::cout<<i<<", "<<j<<" = "<<space<<std::endl;
                if(space < fMinSpacing && std::fabs(space) > fAbsEps){fMinSpacing = space;}
                if(space > fMaxSpacing){fMaxSpacing = space;}
            }
        }

        std::cout<<"start = "<<fStart<<std::endl;
        std::cout<<"min delta = "<<fMinSpacing<<", max delta = "<<fMaxSpacing<<std::endl;

        std::cout<<"fMaxSpace/fMinSpace = "<<fMaxSpacing/fMinSpacing<<std::endl;

        std::size_t grid_pts = (std::size_t) fMaxSpacing/fMinSpacing;

        //check for max deviation of this grid from the points we were given
        double max_dev = 0;
        for(std::size_t i=0; i<npts; i++)
        {   
            double val = fPoints[i];
            std::size_t n_intervals = (std::size_t) (val - fStart)/fMinSpacing;
            double frac = (val - fStart)/fMinSpacing - n_intervals;
            std::cout<<"interval @"<<i<<" = "<<n_intervals<<std::endl;
            std::cout<<"frac @ "<<i<<" = "<<frac<<std::endl;
            if(std::fabs(frac) > max_dev){max_dev = std::fabs(frac);}
        }

        if(max_dev > fEpsilon)
        {
            std::cout<<"err max dev = "<<max_dev<<" eps = "<<fEpsilon<<std::endl;
        }

        // 
        // //msg ("fStart %lf min_space %lf ave_loc %lf", -1, fStart, min_space, ave_loc);
        // div = 1;
        // 
        // bool spacing_err = false;
        // std::map<std::size_t, double> chan_idx_to_mbd_idx;
        // 
        // do                                  /* Yup, its one of them thar do-while loops */
        // {
        //     spacing_ok = 1;
        //     spacing = min_space / div;      /* Sub-divide the spacing  */        
        //     div++;
        //     grid_pts = 2;
        //     for(std::size_t i = 0; i < n_pts; i++)
        //     {
        //         index = (fPoints[i] - fStart ) / fSpacing;
        //                                     /* Check whether all ieqs */
        //                                     /* lie on grid points */
        //         if (fabs(index - (int)(index+0.5)) > fEpsilon){spacing_ok = 0;}
        // 
        //         index = (double) (int)(index + 0.5);
        //                                     /* Make # of grid points the smallest  */
        //         for(int i = 1; i < 8; i++)     /* power of 2 that will cover all points */
        //         {
        //             if( (grid_pts - 1) < index)
        //             {
        //                 grid_pts *= 2;
        //             }
        //         }
        // 
        //         if ((index > (GRID_PTS-1)) || (index < 0))
        //         {
        //             spacing_err = true;
        //             chan_idx_to_mbd_idx[i] = BOGUS_MB_INDEX;
        //             // status.space_err = 1;
        //             // status.mb_index[i] = BOGUS_MB_INDEX;
        //         }
        //         else
        //         {
        //             chan_idx_to_mbd_idx[i] = index;
        //             //status.mb_index[i] = index;
        //         }
        //     }
        // }
        // while ((div < 256) && (spacing_ok == 0));
        // 
        // 
        // std::cout<<"N MBD grid pts = "<<grid_pts<<std::endl;
        // 
        // for(auto it = chan_idx_to_mbd_idx.begin(); it != chan_idx_to_mbd_idx.end(); it++)
        // {
        //     std::cout<<"chan2mbd idx = "<<it->first<<", "<<it->second<<std::endl;
        // }
        // 
        // fSpacing = min_space;
        // fStart = fStart;
        // fNGridPoints = grid_pts;


    }
    else 
    {
        msg_error("math", "cannot construct uniform grid with no points given." << eom);
    }

}



}//end namespace