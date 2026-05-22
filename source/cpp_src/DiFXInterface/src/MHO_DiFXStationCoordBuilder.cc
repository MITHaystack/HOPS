#include "MHO_DiFXStationCoordBuilder.hh"

#include "MHO_DiFXTimeUtilities.hh"
#include "MHO_Message.hh"

#include "difxio/difx_input.h" //MAX_MODEL_ORDER

#include <cmath>
#include <vector>

namespace hops
{

void MHO_DiFXStationCoordBuilder::Extract(std::map< std::string, station_coord_type* >& out)
{
    if(fInput == nullptr)
    {
        msg_error("difx_interface", "MHO_DiFXStationCoordBuilder has no input data" << eom);
        return;
    }

    //populate `out` with one station_coord_type per antenna in the current scan
    //(delay spline, az/el splines, parallactic-angle spline, and u/v/w splines).
    //The phase spline (legacy type_302) is intentionally omitted since nothing
    //downstream of fourfit consumes it.

    const mho_json& input = *fInput;
    std::size_t nAntenna = input["scan"][fScanIndex]["nAntenna"];
    std::size_t nPhaseCenters = input["scan"][fScanIndex]["nPhaseCentres"];
    std::size_t phase_center = 0;
    if(nPhaseCenters > 1)
    {
        msg_warn("difx_interface", "more than one phase center is not supported, using the first. " << eom);
    }

    std::size_t phaseCenterSrcId = input["scan"][fScanIndex]["phsCentreSrcs"][phase_center];
    mho_json src = input["source"][phaseCenterSrcId];
    double src_dec = src["dec"];

    for(std::size_t n = 0; n < nAntenna; n++)
    {
        std::string difx_station_code = input["antenna"][n]["name"];
        station_coord_type* st_coord = new station_coord_type();
        out[difx_station_code] = st_coord;

        mho_json antenna_poly = input["scan"][fScanIndex]["DifxPolyModel"][n][phase_center];
        mho_json ant = input["antenna"][n];
        std::string station_mount = ant["mount"];
        std::vector< double > position = ant["position"];

        int mjd = antenna_poly[0]["mjd"];
        int sec = antenna_poly[0]["sec"];
        std::string model_start = get_vexdate_from_mjd_sec(mjd, sec);
        double duration = antenna_poly[0]["validDuration"];

        std::size_t n_order = antenna_poly[0]["order"];
        std::size_t n_coord = NCOORD;
        std::size_t n_poly = antenna_poly.size();

        st_coord->Resize(n_coord, n_poly, n_order + 1);

        std::get< COORD_AXIS >(*st_coord)[0] = "delay";
        std::get< COORD_AXIS >(*st_coord)[1] = "azimuth";
        std::get< COORD_AXIS >(*st_coord)[2] = "elevation";
        std::get< COORD_AXIS >(*st_coord)[3] = "parallactic_angle";
        std::get< COORD_AXIS >(*st_coord)[4] = "u";
        std::get< COORD_AXIS >(*st_coord)[5] = "v";
        std::get< COORD_AXIS >(*st_coord)[6] = "w";

        for(std::size_t i = 0; i < n_poly; i++)
        {
            std::get< INTERVAL_AXIS >(*st_coord)[i] = i * duration;
        }
        for(std::size_t i = 0; i <= n_order; i++)
        {
            std::get< COEFF_AXIS >(*st_coord)[i] = i;
        }

        for(std::size_t i = 0; i < n_poly; i++)
        {
            mho_json poly_interval = antenna_poly[i];
            for(std::size_t p = 0; p <= n_order; p++)
            {
                double delay = poly_interval["delay"][p];
                double az = poly_interval["az"][p];
                double el = poly_interval["elgeom"][p];
#ifdef CALC_SUPPORTS_PARANGLE //not defined! CALC does not yet support par. angle spline
                double par = poly_interval["parangle"][p];
#else
                double par = 0.0;
#endif
                double u = poly_interval["u"][p];
                double v = poly_interval["v"][p];
                double w = poly_interval["w"][p];

                //negative sign needed to match difx2mark4 convention
                st_coord->at(0, i, p) = -1.0 * MICROSEC_TO_SEC * delay;
                st_coord->at(1, i, p) = az;
                st_coord->at(2, i, p) = el;
                st_coord->at(3, i, p) = par;
                st_coord->at(4, i, p) = u;
                st_coord->at(5, i, p) = v;
                st_coord->at(6, i, p) = w;
            }
        }

        ApplyDelayClockCorrection(ant, antenna_poly, st_coord);

        st_coord->Insert(std::string("model_interval"), duration);
        st_coord->Insert(std::string("model_start"), model_start);
        st_coord->Insert(std::string("mount"), station_mount);
        st_coord->Insert(std::string("X"), position[0]);
        st_coord->Insert(std::string("Y"), position[1]);
        st_coord->Insert(std::string("Z"), position[2]);

        ComputeZerothOrderParallacticAngle(st_coord, position[0], position[1], position[2], src_dec, duration);

        int nsplines = n_poly;
        st_coord->Insert(std::string("nsplines"), nsplines);
    }
}

void MHO_DiFXStationCoordBuilder::ApplyDelayClockCorrection(const mho_json& ant, const mho_json& ant_poly,
                                                            station_coord_type* st_coord)
{
    //see difx2mark4 createType3s.c line 269
    double clock[MAX_MODEL_ORDER + 1];
    //units of difx are usec, ff uses sec; shift clock polynomial to start of model interval
    double clockrefmjd = ant["clockrefmjd"];
    double modelrefmjd = ant_poly[0]["mjd"];
    double modelrefsec = ant_poly[0]["sec"];
    double deltat = 86400.0 * (modelrefmjd - clockrefmjd) + modelrefsec;

    for(std::size_t i = 0; i < std::get< INTERVAL_AXIS >(*st_coord).GetSize(); i++)
    {
        double sec_offset = std::get< INTERVAL_AXIS >(*st_coord)[i];
        double dt = deltat + sec_offset;
        int nclock = GetDifxAntennaShiftedClock(ant, dt, 6, clock);

        //difx delay doesn't have clock added in, so we do it here
        for(int p = 0; p < MAX_MODEL_ORDER + 1; p++)
        {
            if(p < nclock)
            {
                //negative sign matches difx2mark4 convention
                st_coord->at(0, i, p) -= MICROSEC_TO_SEC * clock[p];
            }
        }
    }
}

int MHO_DiFXStationCoordBuilder::GetDifxAntennaShiftedClock(const mho_json& da, double dt, int outputClockSize,
                                                            double* clockOut)
{
    //lifted from difx_antenna.c line 288 with minor changes
    if(!(da.contains("clockorder")) || !(da.contains("clockcoeff")))
    {
        return -1;
    }
    int clockorder = da["clockorder"];
    if(outputClockSize < clockorder + 1)
    {
        return -2;
    }

    double a[MAX_MODEL_ORDER + 1];
    for(int i = 0; i < MAX_MODEL_ORDER + 1; ++i)
    {
        a[i] = 0.0;
        if(i <= clockorder)
        {
            double value = da["clockcoeff"][i];
            a[i] = value;
        }
    }

    double t2 = dt * dt;
    double t3 = t2 * dt;
    double t4 = t2 * t2;
    double t5 = t3 * t2;

    switch(clockorder)
    {
        case 5:
            clockOut[5] = a[5];
        case 4:
            clockOut[4] = a[4] + 5 * a[5] * dt;
        case 3:
            clockOut[3] = a[3] + 4 * a[4] * dt + 10 * a[5] * t2;
        case 2:
            clockOut[2] = a[2] + 3 * a[3] * dt + 6 * a[4] * t2 + 10 * a[5] * t3;
        case 1:
            clockOut[1] = a[1] + 2 * a[2] * dt + 3 * a[3] * t2 + 4 * a[4] * t3 + 5 * a[5] * t4;
        case 0:
            clockOut[0] = a[0] + a[1] * dt + a[2] * t2 + a[3] * t3 + a[4] * t4 + a[5] * t5;
    }

    return clockorder + 1;
}

void MHO_DiFXStationCoordBuilder::ComputeZerothOrderParallacticAngle(station_coord_type* st_coord, double X, double Y, double Z,
                                                                     double dec, double dt)
{
    //adapted from difx2mark4 createType3s.c; constant term only since CALC's parangle spline isn't available yet
    std::size_t n_poly = std::get< INTERVAL_AXIS >(*st_coord).GetSize();
    for(std::size_t i = 0; i < n_poly; i++)
    {
        for(std::size_t p = 0; p < 6; p++)
        {
            if(p == 0)
            {
                double az0 = st_coord->at(1, i, 0);
                double az1 = st_coord->at(1, i, 1);
                double el0 = st_coord->at(2, i, 0);
                double el1 = st_coord->at(2, i, 1);
                double geoc_lat = std::atan2(Z, std::sqrt(X * X + Y * Y));
                double el = (M_PI / 180.0) * (el0 + 0.5 * dt * el1);
                double az = (M_PI / 180.0) * (az0 + 0.5 * dt * az1);
                double sha = -1.0 * std::cos(el) * std::sin(az) / std::cos(dec);
                double cha = (std::sin(el) - std::sin(geoc_lat) * std::sin(dec)) / (std::cos(geoc_lat) * std::cos(dec));
                //approximate (first order in f) geodetic-from-geocentric conversion (magic 1.00674 == 1/(1-e^2) for WGS84)
                double geod_lat = std::atan(1.00674 * std::tan(geoc_lat));
                double par_angle = (180.0 / M_PI) * std::atan2(sha, (std::cos(dec) * std::tan(geod_lat) - std::sin(dec) * cha));
                st_coord->at(3, i, p) = par_angle;
            }
            else
            {
                st_coord->at(3, i, p) = 0.0;
            }
        }
    }
}

} // namespace hops
