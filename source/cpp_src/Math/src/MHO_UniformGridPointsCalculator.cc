#include "MHO_UniformGridPointsCalculator.hh"
#include "MHO_Message.hh"

#include <limits>

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
    fAverageLocation = 0;
    fSpread = 0;
    fDefaultGridPoints = 2;
    fSpacingError = false;
    // #ifdef EXTRA_INTERP_DBG
    // fDefaultGridPoints = 256;
    // #endif
}

MHO_UniformGridPointsCalculator::~MHO_UniformGridPointsCalculator(){};

void MHO_UniformGridPointsCalculator::GetUniquePoints(const std::vector< double >& in_pts, double eps,
                                                      std::vector< double >& out_pts,
                                                      std::map< std::size_t, std::size_t >& index_map) const
{
    out_pts.clear();
    index_map.clear();
    std::size_t point_index = 0;
    out_pts.push_back(in_pts[0]);
    index_map[0] = point_index;
    for(std::size_t idx = 1; idx < in_pts.size(); idx++)
    {
        //check if (only) adjacent points share a value within epsilon
        if(std::fabs(in_pts[idx] - in_pts[idx - 1]) > eps)
        {
            out_pts.push_back(in_pts[idx]);
            point_index++;
        }
        index_map[idx] = point_index;
    }
}

void MHO_UniformGridPointsCalculator::SetPoints(const std::vector< double >& pts)
{
    fPoints.clear();
    fPoints = pts;
}

void MHO_UniformGridPointsCalculator::SetPoints(const double* pts, std::size_t npts)
{
    fPoints.clear();
    fPoints.resize(npts);
    for(std::size_t i = 0; i < npts; i++)
    {
        fPoints[i] = pts[i];
    }
}

void MHO_UniformGridPointsCalculator::Calculate()
{
    int n_pts = 8192; //default
    do
    {
        Calculate_v1(n_pts);
        //crude hack, keep trying if we get a spacing error...
        n_pts *= 2;
        //detect integer overflow and abort
        if(n_pts >= std::numeric_limits< int >::max() / 2)
        {
            Calculate_v1(8192); //go back to default, and abort search
            msg_error("math", "could not determine proper uniform grid for given set of points" << eom);
            break;
        }
    }
    while(fSpacingError);
    //Calculate_v2();
}

void MHO_UniformGridPointsCalculator::Calculate_v1(int max_pts)
{
    //this function is a basic adaptation of freq_spacing (same hard-coded parameters, etc.)
    std::size_t MBD_MAXPTS = max_pts; //default is 8192
    std::size_t MBD_MULT = 4;
    std::size_t MBD_GRID_PTS = MBD_MAXPTS / MBD_MULT;
    std::size_t BOGUS_MBD_INDEX = MBD_GRID_PTS * MBD_MULT + 1;

    fIndexMap.clear();
    if(fPoints.size() != 0)
    {
        int fr, div, grid_pts, spacing_ok;

        std::size_t n_pts;
        double freq, min_pts, min_space, ave_loc, spacing, index, space;

        //find the min freq, the average freq, spacing, etc.
        n_pts = fPoints.size();

        min_pts = std::numeric_limits< double >::max();
        min_space = std::numeric_limits< double >::max();
        ave_loc = 0.0;
        for(std::size_t i = 0; i < n_pts; i++)
        {
            double freq = fPoints[i];
            if(freq < min_pts)
            {
                min_pts = freq;
            }
            ave_loc += freq;
            for(std::size_t j = i + 1; j < n_pts; j++)
            {
                space = std::fabs(fPoints[j] - freq);
                if(space < min_space && space > fAbsEps)
                {
                    min_space = space;
                }
            }
        }
        ave_loc /= (double)n_pts;

        fAverageLocation = ave_loc;

        //determine the spread about the average
        double spread = 0.0;
        for(std::size_t i = 0; i < n_pts; i++)
        {
            double freq = fPoints[i];
            double delta = freq - ave_loc;
            spread += delta * delta;
        }

        if(n_pts > 1)
        {
            spread = std::sqrt(spread / (double)n_pts);
        }
        fSpread = spread;

        div = 1;
        fSpacingError = false;
        std::map< std::size_t, double > chan_idx_to_mbd_idx;
        do
        {
            spacing_ok = 1;
            spacing = min_space / div;
            div++;
            grid_pts = fDefaultGridPoints; //use this value to simplify debugging of MBD search

            for(std::size_t fr = 0; fr < n_pts; fr++)
            {
                index = (fPoints[fr] - min_pts) / spacing;
                // Check whether all freqs lie on grid points
                if(fabs(index - (int)(index + 0.5)) > fEpsilon)
                {
                    spacing_ok = 0;
                }
                index = (double)(int)(index + 0.5);
                // Make # of grid points the smallest power of 2 that will cover all points
                for(int j = 1; j < 8; j++)
                {
                    if((grid_pts - 1) < index)
                    {
                        grid_pts *= 2;
                    }
                }

                if((index > (MBD_GRID_PTS - 1)) || (index < 0))
                {
                    fSpacingError = true;
                    chan_idx_to_mbd_idx[fr] = BOGUS_MBD_INDEX;
                    fIndexMap[fr] = (std::size_t)index;
                }
                else
                {
                    chan_idx_to_mbd_idx[fr] = index;
                }
            }
        }
        while((div < 256) && (spacing_ok == 0));

        for(auto it = chan_idx_to_mbd_idx.begin(); it != chan_idx_to_mbd_idx.end(); it++)
        {
            fIndexMap[it->first] = (std::size_t)it->second;
        }

        fSpacing = spacing;
        fStart = min_pts;

        if(grid_pts > MBD_GRID_PTS)
        {
            grid_pts = MBD_GRID_PTS;
        }
        grid_pts *= MBD_MULT;
        fNGridPoints = grid_pts;
    }
    else
    {
        msg_error("math", "cannot construct uniform grid with no points given." << eom);
    }
}

void MHO_UniformGridPointsCalculator::Calculate_v2()
{

    //this function computes a set of grid points but does not enforce that it be a power of 2 in size
    fIndexMap.clear();
    if(fPoints.size() != 0)
    {
        FindStartAndMinMaxSpacing();

        std::size_t start_grid_pts = (std::size_t)std::round(fMaxSpacing / fMinSpacing);
        std::size_t grid_pts = start_grid_pts;
        double max_delta = 0;
        double grid_spacing = fMinSpacing;
        std::size_t n_attempts = 0;
        double factor = 1;

        do
        {
            grid_pts = factor * start_grid_pts;
            grid_spacing = fMinSpacing / factor;

            max_delta = 0;
            for(std::size_t i = 0; i < fPoints.size(); i++)
            {
                double val = fPoints[i];
                double location = val - fStart;
                //find nearest grid point
                std::size_t idx = std::round(location / grid_spacing);
                fIndexMap[i] = idx;
                double delta = location - idx * grid_spacing;
                if(std::fabs(delta) > max_delta)
                {
                    max_delta = std::fabs(delta);
                }
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

void MHO_UniformGridPointsCalculator::FindStartAndMinMaxSpacing()
{
    if(fPoints.size() != 0)
    {
        fStart = std::numeric_limits< double >::max();
        fMinSpacing = std::numeric_limits< double >::max();
        fMaxSpacing = 0;

        //look for min point and min spacing between any pair of points
        std::size_t npts = fPoints.size();
        for(std::size_t i = 0; i < npts; i++)
        {
            double val = fPoints[i];
            if(val < fStart)
            {
                fStart = val;
            }
            for(std::size_t j = i + 1; j < npts; j++)
            {
                double space = std::fabs(fPoints[j] - val);
                if(space < fMinSpacing && std::fabs(space) > fAbsEps)
                {
                    fMinSpacing = space;
                }
                if(space > fMaxSpacing)
                {
                    fMaxSpacing = space;
                }
            }
        }
    }
}

} // namespace hops
