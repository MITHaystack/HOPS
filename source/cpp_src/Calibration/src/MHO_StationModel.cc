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
    fEvalTimeString = "";
};

MHO_StationModel::~MHO_StationModel(){};

void MHO_StationModel::ComputeModel()
{
    if(fData != nullptr)
    {
        //get the station code
        std::string code = RetrieveTag< std::string >(fData, "station_code");
        msg_debug("calibration", "station code: " << code << eom);

        //get the ref/rem station delay model start times
        std::string model_start = RetrieveTag< std::string >(fData, "model_start");
        //convert string to time point
        auto start = hops_clock::from_vex_format(model_start);

        //if we have an evaluation time, then convert the vex string to a time point
        //otherwise the default is to just use the start time of the model
        auto eval_time = start;
        if(fEvalTimeString == "")
        {
            msg_warn("calibration", "station model evaluation time not set, using model start time" << eom);
            eval_time = hops_clock::from_vex_format(fEvalTimeString);
        }

        msg_debug("calibration", "evaluation time is: " << hops_clock::to_iso8601_format(eval_time) << eom);
        msg_debug("calibration", "model start time is: " << hops_clock::to_iso8601_format(start) << eom);

        //calculate time differences
        auto tdiff_duration = eval_time - start;
        //convert durations to double (seconds)
        double tdiff = std::chrono::duration< double >(tdiff_duration).count();

        //figure out which spline interval overlaps with the fourfit reference time
        double model_interval = RetrieveTag< double >(fData, "model_interval");
        int int_no = std::floor(tdiff / model_interval);
        CheckSplineInterval(fData->GetDimension(INTERVAL_AXIS), tdiff, int_no, code);

        if(int_no < 0)
        {
            msg_error("calibration", "could not determine delay spline interval" << eom);
            return;
        }

        //calculate seconds into target interval
        double dt = tdiff - (int_no * model_interval);

        msg_debug("calibration", "model interval: " << int_no << " and time offset: " << dt << eom);

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

        msg_debug("calibration", "station coord model: (delay, azimuth, elevation, par_angle, u, v, w) = " << eol);
        msg_debug("calibration", "(" << fDelay << ", " << fAzimuth << ", " << fElevation << ", " << fParAngle << ", " << fU
                                     << ", " << fV << ", " << fW << ")." << eom);
    }
    else
    {
        msg_error("calibration", "cannot compute station coordinate model, missing station data. " << eom);
    }
}

void MHO_StationModel::CheckSplineInterval(int n_intervals, double tdiff, int& int_no, std::string station_id)
{
    if(n_intervals == 0)
    {
        msg_error("calibration", "number of spline intervals is 0, missing or malformed data?" << eom);
        int_no = -1;
    }

    if(tdiff < 0.0)
    {
        msg_warn("calibration",
                 "evaluation time is outside of station: " << station_id << " spline range - must extrapolate!" << eom);
        int_no = 0;
    }
    if(int_no >= n_intervals)
    {
        msg_warn("calibration",
                 "evaluation time is outside of station: " << station_id << " spline range - must extrapolate!" << eom);
        int_no = n_intervals - 1;
    }
}

} // namespace hops
