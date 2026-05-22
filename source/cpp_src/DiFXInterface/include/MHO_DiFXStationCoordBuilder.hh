#ifndef MHO_DiFXStationCoordBuilder_HH__
#define MHO_DiFXStationCoordBuilder_HH__

#include <cstddef>
#include <map>
#include <string>

#include "MHO_ContainerDefinitions.hh"
#include "MHO_JSONHeaderWrapper.hh"

namespace hops
{

/*!
 *@file  MHO_DiFXStationCoordBuilder.hh
 *@class MHO_DiFXStationCoordBuilder
 *@author  J. Barrett - barrettj@mit.edu
 *@date Fri May 22 03:56:07 PM EDT 2026
 *@brief Responsible for building the per-station station_coord_type spline data (delay, az, el,
 * parallactic angle, u, v, w) from a DiFX .input JSON. Wraps all the math that was
 * previously inline in MHO_DiFXScanProcessor: DifxPolyModel ingestion, clock
 * polynomial shift+addition, and the zeroth-order parallactic-angle
 * approximation that stands in until CALC's parangle spline is available...probably never.
 *
 * Ownership model: Extract() populates the supplied map with newly-allocated
 * station_coord_type pointers keyed by DiFX (all-caps) station code. The
 * caller takes ownership and is responsible for deletion
 */
class MHO_DiFXStationCoordBuilder
{
    public:
        MHO_DiFXStationCoordBuilder(): fInput(nullptr), fScanIndex(0) {}

        ~MHO_DiFXStationCoordBuilder() = default;

        void SetDiFXInputData(const mho_json* input) { fInput = input; }

        void SetScanIndex(std::size_t idx) { fScanIndex = idx; }

        //build station_coord_type entries for every antenna in the current scan
        void Extract(std::map< std::string, station_coord_type* >& out);

    private:
        //add the clock polynomial (shifted to the model interval start) into the delay
        //row of st_coord. Matches difx2mark4 createType3s.c behavior.
        void ApplyDelayClockCorrection(const mho_json& ant, const mho_json& ant_poly, station_coord_type* st_coord);

        //evaluate the (possibly higher-order) clock polynomial at offset dt and write
        //the shifted polynomial into clockOut. Returns clockorder+1 on success, negative
        //on error (-1 missing fields, -2 buffer too small). Lifted from difxio
        //difx_antenna.c to avoid an extra link dependency.
        int GetDifxAntennaShiftedClock(const mho_json& da, double dt, int outputClockSize, double* clockOut);

        //fill the constant term of the parallactic-angle row using the (d2m4) previous formula
        //(geocentric -> geodetic latitude, az/el midpoint of the spline interval).
        //all higher-order coefficients are zeroed; CALC does not yet emit a parangle spline.
        void ComputeZerothOrderParallacticAngle(station_coord_type* st_coord, double X, double Y, double Z, double src_dec,
                                                double interval_sec);

        const mho_json* fInput;
        std::size_t fScanIndex;

        static constexpr double MICROSEC_TO_SEC = 1e-6;
};

} // namespace hops

#endif /*! end of include guard: MHO_DiFXStationCoordBuilder */
