/**
 * Tests cover:
 * - Happy path with all 7 coordinates (delay, az, el, par_angle, u, v, w)
 * - Correct slot mapping (each Get* returns its own coordinate)
 * - Higher-order polynomial math verification
 * - Interval selection for multi-interval models
 * - Extrapolation before start and after end
 * - Null data (no crash)
 * - Empty evaluation time string defaults to start time
 */

#include <cmath>
#include <iostream>
#include <sstream>

#include "MHO_Clock.hh"
#include "MHO_ContainerDefinitions.hh"
#include "MHO_StationModel.hh"
#include "MHO_Message.hh"
#include "MHO_TestAssertions.hh"

using namespace hops;

/* ---- Reference polynomial evaluator (matches EvaluateSpline) ---- */
static double eval_poly(const double c[], int n_coeff, double t)
{
    double result = 0.0;
    double tp = 1.0;
    for (int p = 0; p < n_coeff; p++) {
        result += c[p] * tp;
        tp *= t;
    }
    return result;
}

/* ---- Fixtures ---- */
static const double MODEL_INTERVAL = 120.0;
static const std::string MODEL_START = "2024y001d00h00m00s";

/**
 * Build a station_coord_type with n_intervals intervals.
 * The caller fills coefficients via SubView.
 */
static station_coord_type* make_station(int n_intervals)
{
    station_coord_type* sta = new station_coord_type();
    sta->Resize(NCOORD, static_cast<std::size_t>(n_intervals), NCOEFF);

    sta->Insert("station_code", std::string("Gs"));
    sta->Insert("model_start", MODEL_START);
    sta->Insert("model_interval", MODEL_INTERVAL);
    return sta;
}

int main()
{
    // Suppress messages to avoid console spam
    MHO_Message::GetInstance().SetMessageLevel(eFatal);

    /* ================================================================
     * Case 1: Compute Model happy path
     * Evaluate at model_start + 60s -> interval 0, dt = 60s
     * ================================================================ */
    {
        int n_intervals = 3;
        auto* sta = make_station(n_intervals);

        // Interval 0 coefficients for all 7 coordinates
        double delay_c[NCOEFF]     = {1.0e-3,  2.0e-6, -1.5e-9, 4.0e-12, -2.0e-15, 5.0e-19};
        double azimuth_c[NCOEFF]   = {1.0,     1.0e-3,  0.0,    0.0,     0.0,      0.0};
        double elev_c[NCOEFF]      = {0.5,     2.0e-3, -1.0e-6, 0.0,     0.0,      0.0};
        double parang_c[NCOEFF]    = {0.1,     1.0e-4,  0.0,    0.0,     0.0,      0.0};
        double u_c[NCOEFF]         = {1.0e6,   1.0e2,   0.0,    0.0,     0.0,      0.0};
        double v_c[NCOEFF]         = {2.0e6,  -5.0e1,   0.0,    0.0,     0.0,      0.0};
        double w_c[NCOEFF]         = {3.0e6,   1.0e1,   0.0,    0.0,     0.0,      0.0};

        auto sv = sta->SubView(0, 0); // delay
        for (int i = 0; i < NCOEFF; i++) sv(i) = delay_c[i];
        sv = sta->SubView(1, 0);      // azimuth
        for (int i = 0; i < NCOEFF; i++) sv(i) = azimuth_c[i];
        sv = sta->SubView(2, 0);      // elevation
        for (int i = 0; i < NCOEFF; i++) sv(i) = elev_c[i];
        sv = sta->SubView(3, 0);      // par_angle
        for (int i = 0; i < NCOEFF; i++) sv(i) = parang_c[i];
        sv = sta->SubView(4, 0);      // u
        for (int i = 0; i < NCOEFF; i++) sv(i) = u_c[i];
        sv = sta->SubView(5, 0);      // v
        for (int i = 0; i < NCOEFF; i++) sv(i) = v_c[i];
        sv = sta->SubView(6, 0);      // w
        for (int i = 0; i < NCOEFF; i++) sv(i) = w_c[i];

        MHO_StationModel model;
        model.SetStationData(sta);

        // Set eval time to model_start + 60s
        auto start = hops_clock::from_vex_format(MODEL_START);
        auto eval_time = start + std::chrono::seconds(60);
        std::string eval_vex = hops_clock::to_vex_format(eval_time);
        model.SetEvaluationTimeVexString(eval_vex);

        model.ComputeModel();

        // Reference values at dt=60
        double dt = 60.0;
        CHECK_CLOSE(model.GetDelay(), eval_poly(delay_c, NCOEFF, dt), 1e-12);
        CHECK_CLOSE(model.GetAzimuth(), eval_poly(azimuth_c, NCOEFF, dt), 1e-9);
        CHECK_CLOSE(model.GetElevation(), eval_poly(elev_c, NCOEFF, dt), 1e-9);
        CHECK_CLOSE(model.GetParallacticAngle(), eval_poly(parang_c, NCOEFF, dt), 1e-9);
        CHECK_CLOSE(model.GetUCoordinate(), eval_poly(u_c, NCOEFF, dt), 1e-5);
        CHECK_CLOSE(model.GetVCoordinate(), eval_poly(v_c, NCOEFF, dt), 1e-5);
        CHECK_CLOSE(model.GetWCoordinate(), eval_poly(w_c, NCOEFF, dt), 1e-5);

        delete sta;
    }

    /* ================================================================
     * Case 2: ComputeModel_CorrectSlotMapping
     * Give only azimuth a non-zero c_0, verify GetAzimuth is non-zero
     * and all others are zero. Then swap to only u having c_0=1234.5.
     * ================================================================ */
    {
        int n_intervals = 1;
        auto* sta = make_station(n_intervals);

        // Sub-test A: only azimuth has c_0 = 1.0
        {
            auto sv = sta->SubView(0, 0); // delay - zero
            for (int i = 0; i < NCOEFF; i++) sv(i) = 0.0;
            sv = sta->SubView(1, 0);      // azimuth
            sv(0) = 1.0;
            for (int i = 1; i < NCOEFF; i++) sv(i) = 0.0;
            sv = sta->SubView(2, 0);      // elev
            for (int i = 0; i < NCOEFF; i++) sv(i) = 0.0;
            sv = sta->SubView(3, 0);      // par
            for (int i = 0; i < NCOEFF; i++) sv(i) = 0.0;
            sv = sta->SubView(4, 0);      // u
            for (int i = 0; i < NCOEFF; i++) sv(i) = 0.0;
            sv = sta->SubView(5, 0);      // v
            for (int i = 0; i < NCOEFF; i++) sv(i) = 0.0;
            sv = sta->SubView(6, 0);      // w
            for (int i = 0; i < NCOEFF; i++) sv(i) = 0.0;

            MHO_StationModel model;
            model.SetStationData(sta);
            model.SetEvaluationTimeVexString(MODEL_START); // dt=0
            model.ComputeModel();

            CHECK_CLOSE(model.GetDelay(), 0.0, 1e-15);
            CHECK_CLOSE(model.GetAzimuth(), 1.0, 1e-15);
            CHECK_CLOSE(model.GetElevation(), 0.0, 1e-15);
            CHECK_CLOSE(model.GetParallacticAngle(), 0.0, 1e-15);
            CHECK_CLOSE(model.GetUCoordinate(), 0.0, 1e-15);
            CHECK_CLOSE(model.GetVCoordinate(), 0.0, 1e-15);
            CHECK_CLOSE(model.GetWCoordinate(), 0.0, 1e-15);
        }

        // Sub-test B: only u has c_0 = 1234.5
        {
            // Fresh station with all zeros except u
            auto* sta2 = make_station(n_intervals);
            for (int coord = 0; coord < NCOORD; coord++) {
                auto sv = sta2->SubView(coord, 0);
                for (int i = 0; i < NCOEFF; i++) sv(i) = 0.0;
            }
            auto sv = sta2->SubView(4, 0); // u
            sv(0) = 1234.5;

            MHO_StationModel model;
            model.SetStationData(sta2);
            model.SetEvaluationTimeVexString(MODEL_START); // dt=0
            model.ComputeModel();

            CHECK_CLOSE(model.GetDelay(), 0.0, 1e-15);
            CHECK_CLOSE(model.GetAzimuth(), 0.0, 1e-15);
            CHECK_CLOSE(model.GetElevation(), 0.0, 1e-15);
            CHECK_CLOSE(model.GetParallacticAngle(), 0.0, 1e-15);
            CHECK_CLOSE(model.GetUCoordinate(), 1234.5, 1e-12);
            CHECK_CLOSE(model.GetVCoordinate(), 0.0, 1e-15);
            CHECK_CLOSE(model.GetWCoordinate(), 0.0, 1e-15);

            delete sta2;
        }

        delete sta;
    }

    /* ================================================================
     * Case 3: ComputeModel_PolynomialMath_HigherOrder
     * All 6 delay coefficients distinct, eval at dt=30s
     * ================================================================ */
    {
        int n_intervals = 1;
        auto* sta = make_station(n_intervals);

        double delay_c[NCOEFF] = {1.0e-3, 2.0e-6, -1.5e-9, 4.0e-12, -2.0e-15, 5.0e-19};

        auto sv = sta->SubView(0, 0); // delay
        for (int i = 0; i < NCOEFF; i++) sv(i) = delay_c[i];

        MHO_StationModel model;
        model.SetStationData(sta);

        // Set eval time to model_start + 30s
        auto start = hops_clock::from_vex_format(MODEL_START);
        auto eval_time = start + std::chrono::seconds(30);
        std::string eval_vex = hops_clock::to_vex_format(eval_time);
        model.SetEvaluationTimeVexString(eval_vex);

        model.ComputeModel();

        double dt = 30.0;
        double expected = eval_poly(delay_c, NCOEFF, dt);
        CHECK_CLOSE(model.GetDelay(), expected, 1e-15);

        delete sta;
    }

    /* ================================================================
     * Case 4: ComputeModel_IntervalSelection
     * 3 intervals, model_interval=120, distinct azimuth c_1 per interval.
     * eval at model_start + 250s -> floor(250/120)=2, interval 2, dt=10s
     * ================================================================ */
    {
        int n_intervals = 3;
        auto* sta = make_station(n_intervals);

        // Azimuth coefficients: [1.0, c_1, 0, 0, 0, 0] with distinct c_1 per interval
        double c1[3] = {1.0e-3, 1.0e-2, 1.0e-1};

        for (int iv = 0; iv < n_intervals; iv++) {
            auto sv = sta->SubView(1, iv); // azimuth
            sv(0) = 1.0;
            sv(1) = c1[iv];
            for (int i = 2; i < NCOEFF; i++) sv(i) = 0.0;
        }

        MHO_StationModel model;
        model.SetStationData(sta);

        // model_start + 250s -> interval 2, dt = 250 - 2*120 = 10s
        auto start = hops_clock::from_vex_format(MODEL_START);
        auto eval_time = start + std::chrono::seconds(250);
        std::string eval_vex = hops_clock::to_vex_format(eval_time);
        model.SetEvaluationTimeVexString(eval_vex);

        model.ComputeModel();

        // Expected: azimuth from interval 2 at dt=10s
        double dt = 10.0;
        double expected = 1.0 + c1[2] * dt; // 1.0 + 0.1*10 = 2.0
        CHECK_CLOSE(model.GetAzimuth(), expected, 1e-9);

        delete sta;
    }

    /* ================================================================
     * Case 5: ComputeModel_BeforeStart_ClampsAndExtrapolates
     * eval time = model_start - 30s -> tdiff < 0, clamps int_no=0
     * dt = -30.0 (NOT clamped, extrapolation)
     * ================================================================ */
    {
        int n_intervals = 3;
        auto* sta = make_station(n_intervals);

        double delay_c[NCOEFF] = {1.0e-3, 2.0e-6, -1.5e-9, 4.0e-12, -2.0e-15, 5.0e-19};
        auto sv = sta->SubView(0, 0);
        for (int i = 0; i < NCOEFF; i++) sv(i) = delay_c[i];

        MHO_StationModel model;
        model.SetStationData(sta);

        // model_start - 30s
        auto start = hops_clock::from_vex_format(MODEL_START);
        auto eval_time = start - std::chrono::seconds(30);
        std::string eval_vex = hops_clock::to_vex_format(eval_time);
        model.SetEvaluationTimeVexString(eval_vex);

        model.ComputeModel();

        double dt = -30.0;
        double expected = eval_poly(delay_c, NCOEFF, dt);
        CHECK_CLOSE(model.GetDelay(), expected, 1e-12);

        delete sta;
    }

    /* ================================================================
     * Case 6: ComputeModel_AfterEnd_ClampsAndExtrapolates
     * eval at model_start + 390s -> floor(390/120)=3 == n_intervals
     * clamps int_no=2, dt = 390 - 2*120 = 150s
     * ================================================================ */
    {
        int n_intervals = 3;
        auto* sta = make_station(n_intervals);

        // Use distinct delay coeffs for interval 2
        double delay_c[NCOEFF] = {1.0e-3, 2.0e-6, -1.5e-9, 4.0e-12, -2.0e-15, 5.0e-19};
        for (int iv = 0; iv < n_intervals; iv++) {
            auto sv = sta->SubView(0, iv);
            for (int i = 0; i < NCOEFF; i++) sv(i) = delay_c[i] * (iv + 1);
        }

        MHO_StationModel model;
        model.SetStationData(sta);

        // model_start + 390s -> interval 2, dt=150s
        auto start = hops_clock::from_vex_format(MODEL_START);
        auto eval_time = start + std::chrono::seconds(390);
        std::string eval_vex = hops_clock::to_vex_format(eval_time);
        model.SetEvaluationTimeVexString(eval_vex);

        model.ComputeModel();

        // Interval 2 coeffs are delay_c * 3, dt=150s
        double scaled_c[NCOEFF];
        for (int i = 0; i < NCOEFF; i++) scaled_c[i] = delay_c[i] * 3.0;
        double dt = 150.0;
        double expected = eval_poly(scaled_c, NCOEFF, dt);
        CHECK_CLOSE(model.GetDelay(), expected, 1e-6); // larger tol for large values

        delete sta;
    }

    /* ================================================================
     * Case 7: ComputeModel_NullData_DoesNotCrash
     * Don't call SetStationData, just ComputeModel - should log error
     * and not crash
     * ================================================================ */
    {
        MHO_StationModel model;
        // Don't set station data - fData remains null
        model.SetEvaluationTimeVexString(MODEL_START);
        model.ComputeModel();

        // All getters should return 0.0 (default values)
        CHECK_CLOSE(model.GetDelay(), 0.0, 1e-15);
        CHECK_CLOSE(model.GetAzimuth(), 0.0, 1e-15);
        CHECK_CLOSE(model.GetElevation(), 0.0, 1e-15);
        CHECK_CLOSE(model.GetParallacticAngle(), 0.0, 1e-15);
        CHECK_CLOSE(model.GetUCoordinate(), 0.0, 1e-15);
        CHECK_CLOSE(model.GetVCoordinate(), 0.0, 1e-15);
        CHECK_CLOSE(model.GetWCoordinate(), 0.0, 1e-15);
    }

    /* ================================================================
     * Case 8: ComputeModel_EmptyEvalTimeString_DefaultsToStart
     * Set station data but don't call SetEvaluationTimeVexString.
     * The bug was fixed: fEvalTimeString != "" check now properly
     * defaults to start time when empty.
     * ================================================================ */
    {
        int n_intervals = 1;
        auto* sta = make_station(n_intervals);

        double delay_c[NCOEFF] = {1.0e-3, 2.0e-6, -1.5e-9, 4.0e-12, -2.0e-15, 5.0e-19};
        auto sv = sta->SubView(0, 0);
        for (int i = 0; i < NCOEFF; i++) sv(i) = delay_c[i];

        MHO_StationModel model;
        model.SetStationData(sta);
        // NOTE: deliberately NOT calling SetEvaluationTimeVexString

        model.ComputeModel();

        // With empty eval time, should default to start time -> dt=0
        // So GetDelay() should return delay_c[0] = 1.0e-3
        CHECK_CLOSE(model.GetDelay(), 1.0e-3, 1e-15);

        delete sta;
    }

    return 0;
}
