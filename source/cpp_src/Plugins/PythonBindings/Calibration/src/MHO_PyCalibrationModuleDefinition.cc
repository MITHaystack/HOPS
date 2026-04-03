#include <pybind11/numpy.h>
#include <pybind11/pybind11.h>
#include <pybind11/stl.h>
namespace py = pybind11;

// Base operator
#include "MHO_Operator.hh"

// Calibration operator headers
#include "MHO_AdhocPhaseCorrection.hh"
#include "MHO_CircularFieldRotationCorrection.hh"
#include "MHO_DCBlock.hh"
#include "MHO_DoubleSidebandChannelLabeler.hh"
#include "MHO_IonosphericPhaseCorrection.hh"
#include "MHO_LinearDParCorrection.hh"
#include "MHO_LSBOffset.hh"
#include "MHO_ManualChannelDelayCorrection.hh"
#include "MHO_ManualChannelPhaseCorrection.hh"
#include "MHO_ManualPolDelayCorrection.hh"
#include "MHO_ManualPolPhaseCorrection.hh"
#include "MHO_MinWeight.hh"
#include "MHO_MixedPolYShift.hh"
#include "MHO_MixedSidebandNormFX.hh"
#include "MHO_MultitonePhaseCorrection.hh"
#include "MHO_Notches.hh"
#include "MHO_Passband.hh"
#include "MHO_PhaseCalibrationTrim.hh"
#include "MHO_PolarizationProductRelabeler.hh"
#include "MHO_PolarizationRelabeler.hh"
#include "MHO_SingleSidebandNormFX.hh"
#include "MHO_StationDelayCorrection.hh"

using namespace hops;

#define PYBIND11_DETAILED_ERROR_MESSAGES

PYBIND11_MODULE(pyMHO_Calibration, m)
{
    m.doc() = "Python bindings for MHO calibration operators. "
              "Import this module before calling get_operator() on a toolbox so that "
              "pybind11 can downcast MHO_Operator* to the correct derived type.";

    // Ensure base modules are loaded first so MHO_Operator is already registered
    py::module_::import("pyMHO_Containers");
    auto operators_mod = py::module_::import("pyMHO_Operators");

    // -------------------------------------------------------------------------
    // AdhocPhaseMode enum (defined at hops namespace scope, not inside a class)
    // -------------------------------------------------------------------------
    py::enum_< AdhocPhaseMode >(m, "AdhocPhaseMode")
        .value("NONE", AdhocPhaseMode::NONE)
        .value("SINEWAVE", AdhocPhaseMode::SINEWAVE)
        .value("POLYNOMIAL", AdhocPhaseMode::POLYNOMIAL)
        .value("PHYLE", AdhocPhaseMode::PHYLE)
        .export_values();

    // -------------------------------------------------------------------------
    // MHO_MinWeight - zeros out weight values below a minimum threshold
    // -------------------------------------------------------------------------
    py::class_< MHO_MinWeight, MHO_Operator >(m, "MHO_MinWeight")
        .def(py::init<>())
        .def("set_min_weight", &MHO_MinWeight::SetMinWeight,
             "set the minimum allowed weight value; values below this are zeroed");

    // -------------------------------------------------------------------------
    // MHO_IonosphericPhaseCorrection - differential ionospheric TEC correction
    // -------------------------------------------------------------------------
    py::class_< MHO_IonosphericPhaseCorrection, MHO_Operator >(m, "MHO_IonosphericPhaseCorrection")
        .def(py::init<>())
        .def("set_differential_tec", &MHO_IonosphericPhaseCorrection::SetDifferentialTEC,
             "set differential ionospheric TEC value (controls phase dispersion)");

    // -------------------------------------------------------------------------
    // MHO_LSBOffset - LSB phase offset for double-sideband channels
    // -------------------------------------------------------------------------
    py::class_< MHO_LSBOffset, MHO_Operator >(m, "MHO_LSBOffset")
        .def(py::init<>())
        .def("set_station_identifier", &MHO_LSBOffset::SetStationIdentifier,
             "set station id: 1-char => mk4 id, 2-char => 2-char station code")
        .def("get_station_identifier", &MHO_LSBOffset::GetStationIdentifier)
        .def("set_lsb_phase_offset", &MHO_LSBOffset::SetLSBPhaseOffset,
             "set LSB phase offset in degrees");

    // -------------------------------------------------------------------------
    // MHO_StationDelayCorrection - per-station cable/clock delay correction
    // -------------------------------------------------------------------------
    py::class_< MHO_StationDelayCorrection, MHO_Operator >(m, "MHO_StationDelayCorrection")
        .def(py::init<>())
        .def("set_station_identifier", &MHO_StationDelayCorrection::SetStationIdentifier,
             "set station id: 1-char => mk4 id, 2-char => 2-char station code")
        .def("get_station_identifier", &MHO_StationDelayCorrection::GetStationIdentifier)
        // .def("set_reference_frequency", &MHO_StationDelayCorrection::SetReferenceFrequency,
        //      "set reference frequency in MHz") // do no expose ref_freq to user!
        .def("set_pc_delay_offset", &MHO_StationDelayCorrection::SetPCDelayOffset,
             "set delay offset in nanoseconds");

    // -------------------------------------------------------------------------
    // MHO_ManualPolPhaseCorrection - manual polarization phase offset
    // -------------------------------------------------------------------------
    py::class_< MHO_ManualPolPhaseCorrection, MHO_Operator >(m, "MHO_ManualPolPhaseCorrection")
        .def(py::init<>())
        .def("set_station_identifier", &MHO_ManualPolPhaseCorrection::SetStationIdentifier,
             "set station id: 1-char => mk4 id, 2-char => 2-char station code")
        .def("get_station_identifier", &MHO_ManualPolPhaseCorrection::GetStationIdentifier)
        .def("set_polarization", &MHO_ManualPolPhaseCorrection::SetPolarization,
             "set polarization string (e.g. 'X', 'Y', 'R', 'L')")
        .def("set_pc_phase_offset", &MHO_ManualPolPhaseCorrection::SetPCPhaseOffset,
             "set phase offset in degrees applied to this polarization");

    // -------------------------------------------------------------------------
    // MHO_ManualPolDelayCorrection - manual polarization delay offset
    // -------------------------------------------------------------------------
    py::class_< MHO_ManualPolDelayCorrection, MHO_Operator >(m, "MHO_ManualPolDelayCorrection")
        .def(py::init<>())
        .def("set_station_identifier", &MHO_ManualPolDelayCorrection::SetStationIdentifier,
             "set station id: 1-char => mk4 id, 2-char => 2-char station code")
        .def("get_station_identifier", &MHO_ManualPolDelayCorrection::GetStationIdentifier)
        .def("set_polarization", &MHO_ManualPolDelayCorrection::SetPolarization,
             "set polarization string (e.g. 'X', 'Y', 'R', 'L')")
        // .def("set_reference_frequency", &MHO_ManualPolDelayCorrection::SetReferenceFrequency,
        //      "set reference frequency in MHz")  // do no expose ref_freq to user!
        .def("set_pc_delay_offset", &MHO_ManualPolDelayCorrection::SetPCDelayOffset,
             "set delay offset in nanoseconds applied to this polarization");

    // -------------------------------------------------------------------------
    // MHO_ManualChannelPhaseCorrection - per-channel manual phase correction
    // -------------------------------------------------------------------------
    py::class_< MHO_ManualChannelPhaseCorrection, MHO_Operator >(m, "MHO_ManualChannelPhaseCorrection")
        .def(py::init<>())
        .def("set_station_identifier", &MHO_ManualChannelPhaseCorrection::SetStationIdentifier,
             "set station id: 1-char => mk4 id, 2-char => 2-char station code")
        .def("get_station_identifier", &MHO_ManualChannelPhaseCorrection::GetStationIdentifier)
        .def("set_polarization", &MHO_ManualChannelPhaseCorrection::SetPolarization,
             "set polarization string (e.g. 'X', 'Y', 'R', 'L')")
        .def("set_channel_to_pc_phase_map", &MHO_ManualChannelPhaseCorrection::SetChannelToPCPhaseMap,
             "set dict mapping channel label strings to phase offsets in degrees");

    // -------------------------------------------------------------------------
    // MHO_ManualChannelDelayCorrection - per-channel manual delay correction
    // -------------------------------------------------------------------------
    py::class_< MHO_ManualChannelDelayCorrection, MHO_Operator >(m, "MHO_ManualChannelDelayCorrection")
        .def(py::init<>())
        .def("set_station_identifier", &MHO_ManualChannelDelayCorrection::SetStationIdentifier,
             "set station id: 1-char => mk4 id, 2-char => 2-char station code")
        .def("get_station_identifier", &MHO_ManualChannelDelayCorrection::GetStationIdentifier)
        .def("set_polarization", &MHO_ManualChannelDelayCorrection::SetPolarization,
             "set polarization string (e.g. 'X', 'Y', 'R', 'L')")
        .def("set_channel_to_pc_delay_map", &MHO_ManualChannelDelayCorrection::SetChannelToPCDelayMap,
             "set dict mapping channel label strings to delay offsets in nanoseconds");

    // -------------------------------------------------------------------------
    // MHO_Notches - zeroes visibilities and weights within frequency notches
    // -------------------------------------------------------------------------
    py::class_< MHO_Notches, MHO_Operator >(m, "MHO_Notches")
        .def(py::init<>())
        .def("set_notch_boundaries",
             &MHO_Notches::SetNotchBoundaries,
             "set notch boundaries as a flat list of (lower, upper) pairs in MHz");

    // -------------------------------------------------------------------------
    // MHO_Passband - selects (or excludes) a frequency range
    // -------------------------------------------------------------------------
    py::class_< MHO_Passband, MHO_Operator >(m, "MHO_Passband")
        .def(py::init<>())
        .def("set_passband", &MHO_Passband::SetPassband, py::arg("first"), py::arg("second"),
             "set passband limits in MHz; if second < first the range is an exclusion band");

    // -------------------------------------------------------------------------
    // MHO_AdhocPhaseCorrection - time/channel-dependent phase correction
    // (SINEWAVE, POLYNOMIAL, or file-based PHYLE modes)
    // -------------------------------------------------------------------------
    py::class_< MHO_AdhocPhaseCorrection, MHO_Operator >(m, "MHO_AdhocPhaseCorrection")
        .def(py::init<>())
        .def("set_mode", &MHO_AdhocPhaseCorrection::SetMode,
             "set correction mode (AdhocPhaseMode.NONE/SINEWAVE/POLYNOMIAL/PHYLE)")
        .def("get_mode", &MHO_AdhocPhaseCorrection::GetMode)
        .def("set_tref", &MHO_AdhocPhaseCorrection::SetTRef,
             "set reference time in seconds from scan start (used by SINEWAVE and POLYNOMIAL)")
        .def("get_tref", &MHO_AdhocPhaseCorrection::GetTRef)
        .def("set_period", &MHO_AdhocPhaseCorrection::SetPeriod,
             "set sinewave period in seconds (SINEWAVE mode)")
        .def("get_period", &MHO_AdhocPhaseCorrection::GetPeriod)
        .def("set_amplitude", &MHO_AdhocPhaseCorrection::SetAmplitude,
             "set sinewave amplitude in radians (SINEWAVE mode)")
        .def("get_amplitude", &MHO_AdhocPhaseCorrection::GetAmplitude)
        .def("set_polynomial_coeffs", &MHO_AdhocPhaseCorrection::SetPolynomialCoeffs,
             "set polynomial coefficients c0..c5 (up to 6; zeta = c0 + c1*t + c2*t^2 + ..., t in seconds from tref)")
        .def("get_polynomial_coeffs",
             [](const MHO_AdhocPhaseCorrection& self) {
                 const double* c = self.GetPolynomialCoeffs();
                 return std::vector< double >(c, c + 6);
             },
             "get polynomial coefficients as a list of 6 values")
        .def("set_ref_adhoc_file", &MHO_AdhocPhaseCorrection::SetRefAdhocFile,
             py::arg("filename"), py::arg("chans"),
             "set reference station adhoc phase file and channel string (PHYLE mode)")
        .def("get_ref_adhoc_file",
             [](const MHO_AdhocPhaseCorrection& self) {
                 std::string fname, chans;
                 self.GetRefAdhocFile(fname, chans);
                 return std::make_pair(fname, chans);
             },
             "get reference station adhoc phase file info as (filename, chans) tuple")
        .def("set_rem_adhoc_file", &MHO_AdhocPhaseCorrection::SetRemAdhocFile,
             py::arg("filename"), py::arg("chans"),
             "set remote station adhoc phase file and channel string (PHYLE mode)")
        .def("get_rem_adhoc_file",
             [](const MHO_AdhocPhaseCorrection& self) {
                 std::string fname, chans;
                 self.GetRemAdhocFile(fname, chans);
                 return std::make_pair(fname, chans);
             },
             "get remote station adhoc phase file info as (filename, chans) tuple");

    // -------------------------------------------------------------------------
    // MHO_MixedPolYShift - 90-degree Y-polarization phase shift for mixed-pol
    // -------------------------------------------------------------------------
    py::class_< MHO_MixedPolYShift, MHO_Operator >(m, "MHO_MixedPolYShift")
        .def(py::init<>())
        .def("set_phase_offset", &MHO_MixedPolYShift::SetPhaseOffset,
             "set Y-pol phase offset in degrees (default 90 deg)");

    // -------------------------------------------------------------------------
    // MHO_LinearDParCorrection - linear delta-parallactic angle correction
    // -------------------------------------------------------------------------
    py::class_< MHO_LinearDParCorrection, MHO_Operator >(m, "MHO_LinearDParCorrection")
        .def(py::init<>())
        .def("set_pol_product_set",
             [](MHO_LinearDParCorrection& self, std::vector< std::string > pp) {
                 self.SetPolProductSet(pp);
             },
             "set the list of pol-product strings present in the data (e.g. ['XX','XY','YX','YY'])")
        .def("set_reference_parallactic_angle", &MHO_LinearDParCorrection::SetReferenceParallacticAngle,
             "set reference station parallactic angle in degrees")
        .def("set_remote_parallactic_angle", &MHO_LinearDParCorrection::SetRemoteParallacticAngle,
             "set remote station parallactic angle in degrees");

}
