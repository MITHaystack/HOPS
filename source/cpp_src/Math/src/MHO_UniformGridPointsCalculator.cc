#include "MHO_UniformGridPointsCalculator.hh"
#include "MHO_Message.hh"


#define EXTRA_INTERP_DBG

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
MHO_UniformGridPointsCalculator::SetPoints(const std::vector<double>& pts)
{
    fPoints.clear();
    fPoints = pts;
}

void 
MHO_UniformGridPointsCalculator::SetPoints(const double* pts, std::size_t npts)
{
    fPoints.clear();
    fPoints.resize(npts);
    for(std::size_t i=0;i<npts;i++){fPoints[i] = pts[i];}
}

void 
MHO_UniformGridPointsCalculator::Calculate()
{
    Calculate_v1();
    //Calculate_v2();
}

void 
MHO_UniformGridPointsCalculator::Calculate_v1()
{
    //this function is a basic adaptation of freq_spacing (same hard-coded parameters, etc.)
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
                if(space < min_space && space > fAbsEps){min_space = space;}
            }
        }
        ave_loc /= (double)n_pts;

        div = 1;
        bool spacing_err = false;
        std::map<std::size_t, double> chan_idx_to_mbd_idx;
        do
        {
            spacing_ok = 1;
            spacing = min_space / div;
            div++;
            #ifdef EXTRA_INTERP_DBG
                #pragma message("TODO FIXME: Warning additional grid points being used in MBD search.")
                grid_pts = 256; //use this value to simplify debugging of MBD search
            #else 
                grid_pts = 2; //this is the original value
            #endif
            for(std::size_t fr = 0; fr < n_pts; fr++)
            {
                index = (fPoints[fr] - min_pts) / spacing;
                // Check whether all freqs lie on grid points 
                if (fabs(index - (int)(index+0.5)) > fEpsilon){spacing_ok = 0;}
                index = (double) (int)(index + 0.5);
                // Make # of grid points the smallest power of 2 that will cover all points 
                for(int i = 1; i < 8; i++)
                {
                    if( (grid_pts - 1) < index){grid_pts *= 2;}
                }

                if ((index > (MBD_GRID_PTS-1)) || (index < 0))
                {
                    spacing_err = true;
                    chan_idx_to_mbd_idx[fr] = BOGUS_MBD_INDEX;
                    fIndexMap[fr] = (std::size_t) index;
                }
                else{ chan_idx_to_mbd_idx[fr] = index; }
            }
        }
        while ((div < 256) && (spacing_ok == 0));

        for(auto it = chan_idx_to_mbd_idx.begin(); it != chan_idx_to_mbd_idx.end(); it++)
        {
            fIndexMap[it->first] = (std::size_t) it->second;
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

    //this function computes a set of grid points but does not enforce that it be a power of 2 in size
    fIndexMap.clear();
    if(fPoints.size() != 0)
    {
        FindStartAndMinMaxSpacing();

        std::size_t start_grid_pts = (std::size_t) std::round(fMaxSpacing/fMinSpacing);
        std::size_t grid_pts = start_grid_pts;
        double max_delta = 0;
        double grid_spacing = fMinSpacing;
        std::size_t n_attempts = 0;
        double factor = 1;

        do 
        {
            grid_pts = factor*start_grid_pts;
            grid_spacing = fMinSpacing/factor;

            max_delta = 0;
            for(std::size_t i=0; i<fPoints.size(); i++)
            {   
                double val = fPoints[i];
                double location = val - fStart;
                //find nearest grid point 
                std::size_t idx = std::round(location/grid_spacing);
                fIndexMap[i] = idx;
                double delta = location - idx*grid_spacing;
                if(std::fabs(delta) > max_delta){max_delta = std::fabs(delta);}
            }

            n_attempts++;
            if(max_delta > fEpsilon)
            {
                factor = n_attempts;
            }
        }
        while(max_delta > fEpsilon && n_attempts < 128);

        fSpacing = grid_spacing;
        fNGridPoints = grid_pts;
    }
    else 
    {
        msg_error("math", "cannot construct uniform grid with no points given." << eom);
    }

}

void 
MHO_UniformGridPointsCalculator::FindStartAndMinMaxSpacing()
{
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
                if(space < fMinSpacing && std::fabs(space) > fAbsEps){fMinSpacing = space;}
                if(space > fMaxSpacing){fMaxSpacing = space;}
            }
        }
    }

}



}//end namespace