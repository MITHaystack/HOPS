#include <chrono>
#include <cmath>
#include <iostream>
#include <string>

#include "MHO_Clock.hh"
#include "MHO_ContainerDefinitions.hh"
#include "MHO_DelayModel.hh"
#include "MHO_Message.hh"
#include "MHO_TestAssertions.hh"

using namespace hops;

static const std::string MODEL_START = "2024y001d00h00m00s";
static const double MODEL_INTERVAL = 120.0;

// build a station with the given per-interval delay-spline coefficients
static station_coord_type* make_station(int n_intervals, const std::string& code, const std::string& start,
                                        double interval, const double coeffs[][NCOEFF])
{
    station_coord_type* sta = new station_coord_type();
    sta->Resize(NCOORD, static_cast< std::size_t >(n_intervals), NCOEFF);
    sta->Insert("station_code", code);
    sta->Insert("model_start", start);
    sta->Insert("model_interval", interval);
    for(int iv = 0; iv < n_intervals; iv++)
    {
        auto sv = sta->SubView(0, iv); // coordinate 0 == delay
        for(int c = 0; c < NCOEFF; c++)
        {
            sv(c) = coeffs[iv][c];
        }
    }
    return sta;
}

// seconds between two vex time strings, computed exactly as ComputeModel does
static double tdiff_seconds(const std::string& start, const std::string& frt)
{
    return std::chrono::duration< double >(hops_clock::from_vex_format(frt) - hops_clock::from_vex_format(start)).count();
}

static std::string reftime_str(const std::string& start, int offset_sec)
{
    return hops_clock::to_vex_format(hops_clock::from_vex_format(start) + std::chrono::seconds(offset_sec));
}

// mirror of EvaluateDelaySpline: results = {delay, rate, accel}
static void eval_dra(const double* c, double t, double out[3])
{
    out[0] = out[1] = out[2] = 0.0;
    for(int p = 0; p < NCOEFF; p++)
    {
        out[0] += c[p] * std::pow(t, p);
        if(p >= 1)
        {
            out[1] += p * c[p] * std::pow(t, p - 1);
        }
        if(p >= 2)
        {
            out[2] += p * (p - 1) * c[p] * std::pow(t, p - 2);
        }
    }
}

// mirror of CheckSplineInterval (sequential clamps, not else-if)
static int select_interval(int n_intervals, double tdiff, int int_no)
{
    if(n_intervals == 0)
    {
        int_no = -1;
    }
    if(tdiff < 0.0 || int_no < 0)
    {
        int_no = 0;
    }
    if(int_no >= n_intervals)
    {
        int_no = n_intervals - 1;
    }
    return int_no;
}

struct ModelResult
{
    double delay, rate, accel;
    double refDelay, refRate, refStationDelay;
};

// independent reference computation mirroring MHO_DelayModel::ComputeModel
static ModelResult reference_model(const double refc[][NCOEFF], int ref_nint, const std::string& ref_start,
                                   const double remc[][NCOEFF], int rem_nint, const std::string& rem_start,
                                   double interval, const std::string& frt_str, double clockOff, double clockRate)
{
    ModelResult r;

    //  primary delay/rate/accel
    double ref_tdiff = tdiff_seconds(ref_start, frt_str);
    double rem_tdiff = tdiff_seconds(rem_start, frt_str);
    int ref_int = select_interval(ref_nint, ref_tdiff, static_cast< int >(std::floor(ref_tdiff / interval)));
    int rem_int = select_interval(rem_nint, rem_tdiff, static_cast< int >(std::floor(rem_tdiff / interval)));
    double ref_t = ref_tdiff - ref_int * interval;
    double rem_t = rem_tdiff - rem_int * interval;

    double rd[3], md[3];
    eval_dra(refc[ref_int], ref_t, rd);
    eval_dra(remc[rem_int], rem_t, md);
    r.delay = md[0] - rd[0];
    r.rate = md[1] - rd[1];
    r.accel = md[2] - rd[2];

    //  secondary reference-station block (clock-corrected, doppler-scaled)
    rd[0] -= clockOff;
    rd[1] -= clockRate;
    rd[0] *= 1.0 - rd[1];
    double doppler = 1.0 - rd[1];

    ref_tdiff = tdiff_seconds(ref_start, frt_str) - rd[0];
    rem_tdiff = tdiff_seconds(rem_start, frt_str) - rd[0]; // note: both use the ref-station delay
    ref_int = select_interval(ref_nint, ref_tdiff, static_cast< int >(std::floor(ref_tdiff / interval)));
    rem_int = select_interval(rem_nint, rem_tdiff, static_cast< int >(std::floor(rem_tdiff / interval)));
    ref_t = ref_tdiff - ref_int * interval;
    rem_t = rem_tdiff - rem_int * interval;

    double rd2[3], md2[3];
    eval_dra(refc[ref_int], ref_t, rd2);
    eval_dra(remc[rem_int], rem_t, md2);
    r.refDelay = md2[0] - rd2[0];
    r.refRate = (md2[1] - rd2[1]) * doppler;
    r.refStationDelay = rd2[0];
    return r;
}

static int check_all(MHO_DelayModel& model, const ModelResult& exp)
{
    CHECK_CLOSE(model.GetDelay(), exp.delay, 1e-12);
    CHECK_CLOSE(model.GetRate(), exp.rate, 1e-12);
    CHECK_CLOSE(model.GetAcceleration(), exp.accel, 1e-12);
    CHECK_CLOSE(model.GetRefDelay(), exp.refDelay, 1e-12);
    CHECK_CLOSE(model.GetRefRate(), exp.refRate, 1e-12);
    CHECK_CLOSE(model.GetRefStationDelay(), exp.refStationDelay, 1e-12);
    return 0;
}

int main(int /*argc*/, char** /*argv*/)
{
    MHO_Message::GetInstance().SetMessageLevel(eFatal);

    // Case 1: happy path, single interval, fourfit reftime inside interval 0
    {
        double refc[1][NCOEFF] = {{1.0e-3, 2.0e-6, -1.5e-9, 4.0e-12, 0.0, 0.0}};
        double remc[1][NCOEFF] = {{1.2e-3, 1.0e-6, 1.0e-9, 0.0, 0.0, 0.0}};
        std::string frt = reftime_str(MODEL_START, 60);

        station_coord_type* ref = make_station(1, "Gs", MODEL_START, MODEL_INTERVAL, refc);
        station_coord_type* rem = make_station(1, "Ef", MODEL_START, MODEL_INTERVAL, remc);

        MHO_DelayModel model;
        model.SetReferenceStationData(ref);
        model.SetRemoteStationData(rem);
        model.SetFourfitReferenceTimeVexString(frt);
        model.SetReferenceStationClockOffset(0.0);
        model.SetReferenceStationClockRate(0.0);
        model.ComputeModel();

        ModelResult exp = reference_model(refc, 1, MODEL_START, remc, 1, MODEL_START, MODEL_INTERVAL, frt, 0.0, 0.0);
        if(check_all(model, exp) != 0)
        {
            return 1;
        }
        delete ref;
        delete rem;
    }

    // Case 2: multi-interval selection -- reftime lands in interval 1
    {
        double refc[3][NCOEFF] = {{1.0e-3, 1.0e-6, 0.0, 0.0, 0.0, 0.0},
                                  {2.0e-3, -3.0e-6, 5.0e-10, 0.0, 0.0, 0.0},
                                  {5.0e-3, 7.0e-6, 0.0, 0.0, 0.0, 0.0}};
        double remc[3][NCOEFF] = {{1.1e-3, 2.0e-6, 0.0, 0.0, 0.0, 0.0},
                                  {2.5e-3, 4.0e-6, -1.0e-9, 0.0, 0.0, 0.0},
                                  {6.0e-3, 1.0e-6, 0.0, 0.0, 0.0, 0.0}};
        std::string frt = reftime_str(MODEL_START, 150); // tdiff=150 -> interval 1, t=30

        station_coord_type* ref = make_station(3, "Gs", MODEL_START, MODEL_INTERVAL, refc);
        station_coord_type* rem = make_station(3, "Ef", MODEL_START, MODEL_INTERVAL, remc);

        MHO_DelayModel model;
        model.SetReferenceStationData(ref);
        model.SetRemoteStationData(rem);
        model.SetFourfitReferenceTimeVexString(frt);
        model.SetReferenceStationClockOffset(0.0);
        model.SetReferenceStationClockRate(0.0);
        model.ComputeModel();

        ModelResult exp = reference_model(refc, 3, MODEL_START, remc, 3, MODEL_START, MODEL_INTERVAL, frt, 0.0, 0.0);
        if(check_all(model, exp) != 0)
        {
            return 1;
        }
        delete ref;
        delete rem;
    }

    // Case 3: extrapolation before the model start (negative tdiff -> clamp to interval 0)
    {
        double refc[2][NCOEFF] = {{1.0e-3, 2.0e-6, 0.0, 0.0, 0.0, 0.0}, {2.0e-3, 1.0e-6, 0.0, 0.0, 0.0, 0.0}};
        double remc[2][NCOEFF] = {{1.5e-3, -1.0e-6, 0.0, 0.0, 0.0, 0.0}, {3.0e-3, 2.0e-6, 0.0, 0.0, 0.0, 0.0}};
        std::string frt = reftime_str(MODEL_START, -30); // tdiff=-30 -> clamp interval 0, t=-30

        station_coord_type* ref = make_station(2, "Gs", MODEL_START, MODEL_INTERVAL, refc);
        station_coord_type* rem = make_station(2, "Ef", MODEL_START, MODEL_INTERVAL, remc);

        MHO_DelayModel model;
        model.SetReferenceStationData(ref);
        model.SetRemoteStationData(rem);
        model.SetFourfitReferenceTimeVexString(frt);
        model.SetReferenceStationClockOffset(0.0);
        model.SetReferenceStationClockRate(0.0);
        model.ComputeModel();

        ModelResult exp = reference_model(refc, 2, MODEL_START, remc, 2, MODEL_START, MODEL_INTERVAL, frt, 0.0, 0.0);
        if(check_all(model, exp) != 0)
        {
            return 1;
        }
        delete ref;
        delete rem;
    }

    // Case 4: extrapolation past the model end (int_no >= n_intervals -> clamp to last)
    {
        double refc[2][NCOEFF] = {{1.0e-3, 2.0e-6, 0.0, 0.0, 0.0, 0.0}, {2.0e-3, 3.0e-6, -1.0e-9, 0.0, 0.0, 0.0}};
        double remc[2][NCOEFF] = {{1.2e-3, 1.0e-6, 0.0, 0.0, 0.0, 0.0}, {2.4e-3, 5.0e-6, 0.0, 0.0, 0.0, 0.0}};
        std::string frt = reftime_str(MODEL_START, 300); // tdiff=300 -> clamp interval 1, t=180

        station_coord_type* ref = make_station(2, "Gs", MODEL_START, MODEL_INTERVAL, refc);
        station_coord_type* rem = make_station(2, "Ef", MODEL_START, MODEL_INTERVAL, remc);

        MHO_DelayModel model;
        model.SetReferenceStationData(ref);
        model.SetRemoteStationData(rem);
        model.SetFourfitReferenceTimeVexString(frt);
        model.SetReferenceStationClockOffset(0.0);
        model.SetReferenceStationClockRate(0.0);
        model.ComputeModel();

        ModelResult exp = reference_model(refc, 2, MODEL_START, remc, 2, MODEL_START, MODEL_INTERVAL, frt, 0.0, 0.0);
        if(check_all(model, exp) != 0)
        {
            return 1;
        }
        delete ref;
        delete rem;
    }

    // Case 5: non-zero clocks + distinct ref/rem model_start -> exercises the
    // reference-station block (GetRefDelay/GetRefRate/GetRefStationDelay)
    {
        const std::string rem_start = "2024y001d00h00m30s"; // 30 s after ref start
        const double clockOff = 1.0e-6;
        const double clockRate = 2.0e-9;
        double refc[1][NCOEFF] = {{1.0e-3, 2.0e-6, -1.5e-9, 0.0, 0.0, 0.0}};
        double remc[1][NCOEFF] = {{1.2e-3, 1.0e-6, 0.0, 0.0, 0.0, 0.0}};
        std::string frt = reftime_str(MODEL_START, 60);

        station_coord_type* ref = make_station(1, "Gs", MODEL_START, MODEL_INTERVAL, refc);
        station_coord_type* rem = make_station(1, "Ef", rem_start, MODEL_INTERVAL, remc);

        MHO_DelayModel model;
        model.SetReferenceStationData(ref);
        model.SetRemoteStationData(rem);
        model.SetFourfitReferenceTimeVexString(frt);
        model.SetReferenceStationClockOffset(clockOff);
        model.SetReferenceStationClockRate(clockRate);
        model.ComputeModel();

        ModelResult exp =
            reference_model(refc, 1, MODEL_START, remc, 1, rem_start, MODEL_INTERVAL, frt, clockOff, clockRate);
        if(check_all(model, exp) != 0)
        {
            return 1;
        }
        delete ref;
        delete rem;
    }

    // Case 6: missing station data -> ComputeModel is a no-op, primary outputs stay 0
    {
        MHO_DelayModel model;
        model.SetFourfitReferenceTimeVexString(reftime_str(MODEL_START, 60));
        model.ComputeModel(); // both station pointers null -> early-out branch
        CHECK_CLOSE(model.GetDelay(), 0.0, 1e-15);
        CHECK_CLOSE(model.GetRate(), 0.0, 1e-15);
        CHECK_CLOSE(model.GetAcceleration(), 0.0, 1e-15);
    }

    return 0;
}
