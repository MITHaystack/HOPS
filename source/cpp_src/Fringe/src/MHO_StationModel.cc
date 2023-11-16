#include "MHO_StationModel.hh"
#include "MHO_Clock.hh"

#define DELAY_INDEX 0
#define AZIMUTH_INDEX 1
#define ELEVATION_INDEX 2
#define PARANGLE_INDEX 3
#define U_INDEX 4
#define V_INDEX 5
#define W_INDEX 6

namespace hops
{

MHO_StationModel::MHO_StationModel()
{
    fData = nullptr;
    fDelay = 0.0;
    fAzimuth = 0.0;
    fElevation = 0.0;
    fParAngle = 0.0;
    fU = 0.0;
    fV = 0.0;
    fW = 0.0;
};

MHO_StationModel::~MHO_StationModel(){};


void
MHO_StationModel::ComputeModel()
{
    if(fData !=nullptr)
    {
        //convert time string to time point
        auto eval_time = hops_clock::from_vex_format(fEvalTimeString);

        //get the station code
        std::string code = RetrieveTag<std::string>(fData, "station_code");
        msg_debug("fringe", "staion code: " << code << eom );

        //get the ref/rem station delay model start times
        std::string model_start = RetrieveTag<std::string>(fData, "model_start");
        //convert string to time point
        auto start = hops_clock::from_vex_format(model_start);

        msg_debug("fringe", "evaluation time is: "<< hops_clock::to_iso8601_format(eval_time)<< eom);
        msg_debug("fringe", "model start time is: "<<hops_clock::to_iso8601_format(start)<< eom);

        //calculate time differences
        auto tdiff_duration = eval_time - start;
        //convert durations to double (seconds)
        double tdiff = std::chrono::duration<double>(tdiff_duration).count();

        //figure out which spline interval overlaps with the fourfit reference time
        double model_interval = RetrieveTag<double>(fData, "model_interval");
        int int_no = std::floor(tdiff/model_interval);
        CheckSplineInterval(fData->GetDimension(INTERVAL_AXIS), tdiff, int_no, code);

        //calculate seconds into target interval
        double dt = tdiff - (int_no * model_interval);

        msg_debug("fringe", "model interval: "<< int_no <<" and time offset: "<< dt << eom);

        //evaluate the station model: delay, azimuth, elevation, par_angle, u, v, w
        auto dcoeff = fData->SubView(DELAY_INDEX, int_no); //extract spline coeffs
        EvaluateSpline(dcoeff, dt, fDelay);
        
        auto acoeff = fData->SubView(AZIMUTH_INDEX, int_no); //extract spline coeffs
        EvaluateSpline(acoeff, dt, fAzimuth);
        
        auto ecoeff = fData->SubView(ELEVATION_INDEX, int_no); //extract spline coeffs
        EvaluateSpline(ecoeff, dt, fElevation);
        
        //this evaluation doesn't really work, since CALC does not provide a spline for 
        //parallactic angle...this should be calculated by az/el, station coords, etc.
        auto pcoeff = fData->SubView(PARANGLE_INDEX, int_no); //extract spline coeffs
        EvaluateSpline(pcoeff, dt, fParAngle);
        
        auto ucoeff = fData->SubView(U_INDEX, int_no); //extract spline coeffs
        EvaluateSpline(ucoeff, dt, fU);
        
        auto vcoeff = fData->SubView(V_INDEX, int_no); //extract spline coeffs
        EvaluateSpline(vcoeff, dt, fV);

        auto wcoeff = fData->SubView(W_INDEX, int_no); //extract spline coeffs
        EvaluateSpline(wcoeff, dt, fW);

        msg_debug("fringe", "station coord model: (delay, azimuth, elevation, par_angle, u, v, w) = (" <<
            fDelay <<", "<<
            fAzimuth <<", "<<
            fElevation <<", "<<
            fParAngle <<", "<<
            fU <<", "<<
            fV <<", "<<
            fW <<", "<< eom);

    }
    else
    {
        msg_fatal("fringe", "cannot compute station coordinate model, missing station data. " << eom );
        std::exit(1);
    }
}

void
MHO_StationModel::CheckSplineInterval(int n_intervals, double tdiff, int& int_no, std::string station_id)
{
    if(n_intervals == 0)
    {
        msg_fatal("fringe", "number of spline intervals is 0, missing or malformed data?" << eom );
        std::exit(1);
    }

    if(tdiff < 0.0)
    {
        msg_warn("fringe", "evaluation time is outside of station: "<<station_id<<" spline range - must extrapolate!" << eom);
        int_no = 0;
    }
    if(int_no >= n_intervals )
    {
        msg_warn("fringe", "evaluation time is outside of station: "<<station_id<<" spline range - must extrapolate!" << eom);
        int_no = n_intervals-1;
    }
}


}//end namespace
