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
    // MHO_DCBlock - zeros the DC spectral point of each channel
    // No configuration setters; just initialize/execute (inherited from MHO_Operator)
    // -------------------------------------------------------------------------
    py::class_< MHO_DCBlock, MHO_Operator >(m, "MHO_DCBlock")
        .def(py::init<>());

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
             "set differential TEC value (controls phase dispersion)");

    // -------------------------------------------------------------------------
    // MHO_LSBOffset - LSB phase offset for double-sideband channels
    // -------------------------------------------------------------------------
    py::class_< MHO_LSBOffset, MHO_Operator >(m, "MHO_LSBOffset")
        .def(py::init<>())
        .def("set_station_identifier", &MHO_LSBOffset::SetStationIdentifier,
             "set station id: 1-char => mk4 id, 2-char => 2-char station code")
        .def("set_lsb_phase_offset", &MHO_LSBOffset::SetLSBPhaseOffset,
             "set LSB phase offset in degrees");

    // -------------------------------------------------------------------------
    // MHO_StationDelayCorrection - per-station cable/clock delay correction
    // -------------------------------------------------------------------------
    py::class_< MHO_StationDelayCorrection, MHO_Operator >(m, "MHO_StationDelayCorrection")
        .def(py::init<>())
        .def("set_station_identifier", &MHO_StationDelayCorrection::SetStationIdentifier,
             "set station id: 1-char => mk4 id, 2-char => 2-char station code")
        .def("set_reference_frequency", &MHO_StationDelayCorrection::SetReferenceFrequency,
             "set reference frequency in MHz")
        .def("set_pc_delay_offset", &MHO_StationDelayCorrection::SetPCDelayOffset,
             "set delay offset in nanoseconds");

    // -------------------------------------------------------------------------
    // MHO_ManualPolPhaseCorrection - manual polarization phase offset
    // -------------------------------------------------------------------------
    py::class_< MHO_ManualPolPhaseCorrection, MHO_Operator >(m, "MHO_ManualPolPhaseCorrection")
        .def(py::init<>())
        .def("set_station_identifier", &MHO_ManualPolPhaseCorrection::SetStationIdentifier,
             "set station id: 1-char => mk4 id, 2-char => 2-char station code")
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
        .def("set_polarization", &MHO_ManualPolDelayCorrection::SetPolarization,
             "set polarization string (e.g. 'X', 'Y', 'R', 'L')")
        .def("set_reference_frequency", &MHO_ManualPolDelayCorrection::SetReferenceFrequency,
             "set reference frequency in MHz")
        .def("set_pc_delay_offset", &MHO_ManualPolDelayCorrection::SetPCDelayOffset,
             "set delay offset in nanoseconds applied to this polarization");

    // -------------------------------------------------------------------------
    // MHO_ManualChannelPhaseCorrection - per-channel manual phase correction
    // -------------------------------------------------------------------------
    py::class_< MHO_ManualChannelPhaseCorrection, MHO_Operator >(m, "MHO_ManualChannelPhaseCorrection")
        .def(py::init<>())
        .def("set_station_identifier", &MHO_ManualChannelPhaseCorrection::SetStationIdentifier,
             "set station id: 1-char => mk4 id, 2-char => 2-char station code")
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
             "set notch boundaries as a flat list of (lower, upper) pairs in MHz")
        .def("set_weights",
             &MHO_Notches::SetWeights,
             py::keep_alive< 1, 2 >(),
             "set associated weight array (notched weights are also zeroed)");

    // -------------------------------------------------------------------------
    // MHO_Passband - selects (or excludes) a frequency range
    // -------------------------------------------------------------------------
    py::class_< MHO_Passband, MHO_Operator >(m, "MHO_Passband")
        .def(py::init<>())
        .def("set_passband", &MHO_Passband::SetPassband, py::arg("first"), py::arg("second"),
             "set passband limits in MHz; if second < first the range is an exclusion band")
        .def("set_weights",
             &MHO_Passband::SetWeights,
             py::keep_alive< 1, 2 >(),
             "set associated weight array");

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

    // -------------------------------------------------------------------------
    // MHO_CircularFieldRotationCorrection - circular polarization field rotation
    // -------------------------------------------------------------------------
    py::class_< MHO_CircularFieldRotationCorrection, MHO_Operator >(m, "MHO_CircularFieldRotationCorrection")
        .def(py::init<>())
        .def("set_pol_product_set",
             [](MHO_CircularFieldRotationCorrection& self, std::vector< std::string > pp) {
                 self.SetPolProductSet(pp);
             },
             "set the list of pol-product strings present in the data (e.g. ['RR','RL','LR','LL'])")
        .def("set_fourfit_reference_time_vex_string",
             &MHO_CircularFieldRotationCorrection::SetFourfitReferenceTimeVexString,
             "set the fourfit reference time as a VEX-format string")
        .def("set_reference_mount_type", &MHO_CircularFieldRotationCorrection::SetReferenceMountType,
             "set reference station mount type: 'no_mount', 'cassegrain', 'nasmythleft', or 'nasmythright'")
        .def("set_remote_mount_type", &MHO_CircularFieldRotationCorrection::SetRemoteMountType,
             "set remote station mount type: 'no_mount', 'cassegrain', 'nasmythleft', or 'nasmythright'")
        .def("set_reference_station_coordinate_data",
             &MHO_CircularFieldRotationCorrection::SetReferenceStationCoordinateData,
             py::keep_alive< 1, 2 >(),
             "set reference station coordinate data object")
        .def("set_remote_station_coordinate_data",
             &MHO_CircularFieldRotationCorrection::SetRemoteStationCoordinateData,
             py::keep_alive< 1, 2 >(),
             "set remote station coordinate data object");

    // -------------------------------------------------------------------------
    // MHO_SingleSidebandNormFX - SSB freq-to-delay transform (NormFX)
    // MHO_NormFX (abstract intermediate) is intentionally skipped;
    // MHO_Operator is used as the Python parent.  set_weights() is the only
    // user-configurable method beyond initialize()/execute().
    // -------------------------------------------------------------------------
    py::class_< MHO_SingleSidebandNormFX, MHO_Operator >(m, "MHO_SingleSidebandNormFX")
        .def(py::init<>())
        .def("set_weights",
             &MHO_SingleSidebandNormFX::SetWeights,
             py::keep_alive< 1, 2 >(),
             "set weight array used during the NormFX transform");

    // -------------------------------------------------------------------------
    // MHO_MixedSidebandNormFX - mixed-sideband freq-to-delay transform (NormFX)
    // -------------------------------------------------------------------------
    py::class_< MHO_MixedSidebandNormFX, MHO_Operator >(m, "MHO_MixedSidebandNormFX")
        .def(py::init<>())
        .def("set_weights",
             &MHO_MixedSidebandNormFX::SetWeights,
             py::keep_alive< 1, 2 >(),
             "set weight array used during the NormFX transform");

    // -------------------------------------------------------------------------
    // MHO_MultitonePhaseCorrection - multi-tone phase calibration correction
    // -------------------------------------------------------------------------
    py::class_< MHO_MultitonePhaseCorrection, MHO_Operator >(m, "MHO_MultitonePhaseCorrection")
        .def(py::init<>())
        .def("set_station", &MHO_MultitonePhaseCorrection::SetStation,
             "set 2-char station code")
        .def("set_station_mk4id", &MHO_MultitonePhaseCorrection::SetStationMk4ID,
             "set 1-char MK4 station id")
        .def("set_pc_period", &MHO_MultitonePhaseCorrection::SetPCPeriod,
             "set phase-cal averaging period in accumulation periods (APs)")
        .def("set_multitone_pc_data",
             &MHO_MultitonePhaseCorrection::SetMultitonePCData,
             py::keep_alive< 1, 2 >(),
             "set the multi-tone pcal data object")
        .def("set_weights",
             &MHO_MultitonePhaseCorrection::SetWeights,
             py::keep_alive< 1, 2 >(),
             "set the visibility weight array");

    // -------------------------------------------------------------------------
    // MHO_PhaseCalibrationTrim - trims pcal time range to match visibility data
    // Operates on multitone_pcal_type (not visibility_type).
    // -------------------------------------------------------------------------
    py::class_< MHO_PhaseCalibrationTrim, MHO_Operator >(m, "MHO_PhaseCalibrationTrim")
        .def(py::init<>())
        .def("set_visibilities",
             &MHO_PhaseCalibrationTrim::SetVisibilities,
             py::keep_alive< 1, 2 >(),
             "set visibility array whose time range defines the trim target");

    // -------------------------------------------------------------------------
    // Template instantiations
    // -------------------------------------------------------------------------

    // MHO_PolarizationProductRelabeler<visibility_type>
    // Swaps polarization labels within pol-product strings of the visibility array
    py::class_< MHO_PolarizationProductRelabeler< visibility_type >, MHO_Operator >(
        m, "MHO_PolarizationProductRelabeler_Vis")
        .def(py::init<>())
        .def("set_polarization_swap_pair",
             &MHO_PolarizationProductRelabeler< visibility_type >::SetPolarizationSwapPair,
             py::arg("pol1"), py::arg("pol2"),
             "set the two single-character polarization labels to swap (e.g. 'X','Y')")
        .def("set_station_identifier",
             &MHO_PolarizationProductRelabeler< visibility_type >::SetStationIdentifier,
             "set station id: 1-char => mk4 id, 2-char => 2-char station code");

    // MHO_PolarizationProductRelabeler<weight_type>
    py::class_< MHO_PolarizationProductRelabeler< weight_type >, MHO_Operator >(
        m, "MHO_PolarizationProductRelabeler_Wt")
        .def(py::init<>())
        .def("set_polarization_swap_pair",
             &MHO_PolarizationProductRelabeler< weight_type >::SetPolarizationSwapPair,
             py::arg("pol1"), py::arg("pol2"),
             "set the two single-character polarization labels to swap (e.g. 'X','Y')")
        .def("set_station_identifier",
             &MHO_PolarizationProductRelabeler< weight_type >::SetStationIdentifier,
             "set station id: 1-char => mk4 id, 2-char => 2-char station code");

    // MHO_PolarizationRelabeler<multitone_pcal_type>
    // Swaps polarization labels of the pcal pol axis
    py::class_< MHO_PolarizationRelabeler< multitone_pcal_type >, MHO_Operator >(
        m, "MHO_PolarizationRelabeler_PCal")
        .def(py::init<>())
        .def("set_polarization_swap_pair",
             &MHO_PolarizationRelabeler< multitone_pcal_type >::SetPolarizationSwapPair,
             py::arg("pol1"), py::arg("pol2"),
             "set the two single-character polarization labels to swap (e.g. 'X','Y')")
        .def("set_station_identifier",
             &MHO_PolarizationRelabeler< multitone_pcal_type >::SetStationIdentifier,
             "set station id: 1-char => mk4 id, 2-char => 2-char station code");

    // MHO_DoubleSidebandChannelLabeler<visibility_type>
    // Labels paired LSB/USB channels as double-sideband
    py::class_< MHO_DoubleSidebandChannelLabeler< visibility_type >, MHO_Operator >(
        m, "MHO_DoubleSidebandChannelLabeler_Vis")
        .def(py::init<>())
        .def("set_tolerance", &MHO_DoubleSidebandChannelLabeler< visibility_type >::SetTolerance,
             "set frequency-matching tolerance in MHz (default 1e-6)");
}
