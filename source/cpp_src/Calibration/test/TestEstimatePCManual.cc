// MHO_EstimatePCManual is an MHO_InspectingOperator: it reads the visibilities
// (and phasors/weights/plot-data/parameter-store) and emits manual phase-cal
// estimates, but must never mutate its input. All of its internal routines
// (est_phases / est_delays / est_offset / get_manual_phasecal) are protected but
// are selected entirely by the "est_pc_manual" mode in the parameter store:
//   - sign of mode  -> reference (mode>0) vs remote (mode<0) station
//   - bit 0x001     -> est_phases
//   - bits 0x13e    -> est_delays
//   - bit 0x040     -> est_offset
//   - bit 0x080     -> phase-bias ("keep") variant of est_phases

#include <complex>
#include <iostream>
#include <string>

#include "MHO_Constants.hh"
#include "MHO_ContainerDefinitions.hh"
#include "MHO_EstimatePCManual.hh"
#include "MHO_Message.hh"
#include "MHO_TestAssertions.hh"

using namespace hops;

// The internal routines are protected (not private), so a plain subclass can access them
class TestableEstimatePCManual: public MHO_EstimatePCManual
{
    public:
        double call_get_manual_phasecal(int is_remote, int channel_idx, std::string pol)
        {
            return get_manual_phasecal(is_remote, channel_idx, pol);
        }
};

// minimal visibility fixture with channel + phase labels
static void build_vis(visibility_type& vis)
{
    vis.Resize(1, 2, 2, 2); // 1 pol, 2 chan, 2 ap, 2 spec

    auto& pp_ax = std::get< POLPROD_AXIS >(vis);
    pp_ax.at(0) = "XX";

    auto& chan_ax = std::get< CHANNEL_AXIS >(vis);
    chan_ax.at(0) = 8000.0;
    chan_ax.at(1) = 8200.0;
    chan_ax.InsertIndexLabelKeyValue(0, "channel_label", std::string("a"));
    chan_ax.InsertIndexLabelKeyValue(1, "channel_label", std::string("b"));

    // phase labels consumed by get_manual_phasecal
    pp_ax.InsertIndexLabelKeyValue(0, "ref_pcphase_offset_X", 0.20); // rad
    chan_ax.InsertIndexLabelKeyValue(0, "ref_pcphase_X", 0.10);      // rad

    for(std::size_t pp = 0; pp < vis.GetDimension(POLPROD_AXIS); pp++)
        for(std::size_t ch = 0; ch < vis.GetDimension(CHANNEL_AXIS); ch++)
            for(std::size_t ap = 0; ap < vis.GetDimension(TIME_AXIS); ap++)
                for(std::size_t sp = 0; sp < vis.GetDimension(FREQ_AXIS); sp++)
                    vis(pp, ch, ap, sp) = std::complex< double >(1.0, 0.0);
}

static void build_pstore(MHO_ParameterStore& pstore, int mode)
{
    pstore.Set("/files/root_file", std::string("/tmp/fake.root"));
    pstore.Set("/ref_station/site_id", std::string("Gs"));
    pstore.Set("/rem_station/site_id", std::string("Wf"));
    pstore.Set("/config/polprod", std::string("XX"));
    pstore.Set("/control/config/ref_freq", 8000.0);
    pstore.Set("/fringe/resid_phase", 0.5);
    pstore.Set("/control/config/est_pc_manual", mode);
}

static void build_phasors(phasor_type& phasors)
{
    phasors.Resize(3, 2); // 3 channels (incl "All"), 2 APs
    auto& ch_ax = std::get< 0 >(phasors);
    ch_ax.at(0) = 8000.0;
    ch_ax.at(1) = 8200.0;
    ch_ax.at(2) = 0.0;
    ch_ax.InsertIndexLabelKeyValue(0, "channel_label", std::string("a"));
    ch_ax.InsertIndexLabelKeyValue(1, "channel_label", std::string("b"));
    ch_ax.InsertIndexLabelKeyValue(2, "channel_label", std::string("All"));
    for(std::size_t i = 0; i < 3; i++)
        for(std::size_t j = 0; j < 2; j++)
            phasors(i, j) = std::complex< double >(1.0, 0.0);
}

static void build_weights(weight_type& weights)
{
    weights.Resize(1, 2, 2, 1);
    for(std::size_t pp = 0; pp < weights.GetDimension(POLPROD_AXIS); pp++)
        for(std::size_t ch = 0; ch < weights.GetDimension(CHANNEL_AXIS); ch++)
            for(std::size_t ap = 0; ap < weights.GetDimension(TIME_AXIS); ap++)
                weights(pp, ch, ap, 0) = 1.0;
}

// compare two visibility arrays element-wise; returns 0 if identical
static int vis_equal(const visibility_type& a, const visibility_type& b)
{
    for(std::size_t pp = 0; pp < a.GetDimension(POLPROD_AXIS); pp++)
        for(std::size_t ch = 0; ch < a.GetDimension(CHANNEL_AXIS); ch++)
            for(std::size_t ap = 0; ap < a.GetDimension(TIME_AXIS); ap++)
                for(std::size_t sp = 0; sp < a.GetDimension(FREQ_AXIS); sp++)
                {
                    CHECK_CLOSE(a(pp, ch, ap, sp).real(), b(pp, ch, ap, sp).real(), 1e-15);
                    CHECK_CLOSE(a(pp, ch, ap, sp).imag(), b(pp, ch, ap, sp).imag(), 1e-15);
                }
    return 0;
}

// Drive the operator for a given mode (and optional per-station pc_mode overrides)
// through the public interface, then verify Execute succeeded and the input
// visibilities are unchanged (the InspectingOperator contract).
static int run_mode(int mode, const char* ref_pcmode, const char* rem_pcmode)
{
    visibility_type vis;
    build_vis(vis);
    visibility_type pristine;
    pristine.Copy(vis);

    MHO_ParameterStore pstore;
    build_pstore(pstore, mode);
    if(ref_pcmode != nullptr)
    {
        pstore.Set("/control/station/Gs/pc_mode", std::string(ref_pcmode));
    }
    if(rem_pcmode != nullptr)
    {
        pstore.Set("/control/station/Wf/pc_mode", std::string(rem_pcmode));
    }

    mho_json empty_plot;
    phasor_type phasors;
    build_phasors(phasors);
    weight_type weights;
    build_weights(weights);

    MHO_EstimatePCManual op;
    op.SetParameterStore(&pstore);
    op.SetPlotData(empty_plot);
    op.SetPhasors(&phasors);
    op.SetWeights(&weights);
    op.SetArgs(&vis);

    REQUIRE(op.Initialize());
    REQUIRE(op.Execute());
    REQUIRE(vis_equal(vis, pristine) == 0);
    return 0;
}

// test of get_manual_phasecal arithmetic via protected access.
// Per-channel phase = (pol_offset + channel_phase) * rad_to_deg, each term only
// added when its label is present on the corresponding axis.
static int test_get_manual_phasecal()
{
    visibility_type vis;
    build_vis(vis); // sets ref_pcphase_offset_X=0.20 (pp0), ref_pcphase_X=0.10 (chan0)

    MHO_ParameterStore pstore;
    build_pstore(pstore, 0);
    phasor_type phasors;
    build_phasors(phasors);
    weight_type weights;
    build_weights(weights);

    TestableEstimatePCManual op;
    op.SetParameterStore(&pstore);
    op.SetPhasors(&phasors);
    op.SetWeights(&weights);
    op.SetArgs(&vis);
    REQUIRE(op.Initialize());
    REQUIRE(op.Execute()); // mode 0 -> sets fVisibilities, no estimation performed

    // ref, channel 0: both offset (0.20) and channel (0.10) labels present
    CHECK_CLOSE(op.call_get_manual_phasecal(0, 0, "X"), 0.30 * MHO_Constants::rad_to_deg, 1e-6);
    // ref, channel 1: only the pol offset (0.20) is present
    CHECK_CLOSE(op.call_get_manual_phasecal(0, 1, "X"), 0.20 * MHO_Constants::rad_to_deg, 1e-6);
    // remote: no rem_* labels present -> 0
    CHECK_CLOSE(op.call_get_manual_phasecal(1, 0, "X"), 0.0, 1e-15);
    return 0;
}

int main(int /*argc*/, char** /*argv*/)
{
    MHO_Message::GetInstance().SetMessageLevel(eFatal);

    // mode 0: no sub-routine selected -> est_pc_manual sets up and returns
    REQUIRE(run_mode(0, nullptr, nullptr) == 0);

    // guard: a station not in pc_mode "manual" -> early return before any routine
    REQUIRE(run_mode(0x001, "multitone", nullptr) == 0); // ref non-manual
    REQUIRE(run_mode(0x001, nullptr, "multitone") == 0); // rem non-manual

    // explicit "manual" on both stations exercises the have_refmode/have_remmode paths
    REQUIRE(run_mode(0, "manual", "manual") == 0);

    // est_phases on the reference (mode>0) and remote (mode<0) station
    REQUIRE(run_mode(0x001, nullptr, nullptr) == 0);
    REQUIRE(run_mode(-0x001, nullptr, nullptr) == 0);

    // est_phases phase-bias ("keep") variant (0x080 bit set alongside 0x001)
    REQUIRE(run_mode(0x081, nullptr, nullptr) == 0);

    // est_offset on the reference (mode +0x040) and remote (mode -0x040) station
    REQUIRE(run_mode(0x040, nullptr, nullptr) == 0);
    REQUIRE(run_mode(-0x040, nullptr, nullptr) == 0);

    //get_manual_phasecal accumulation arithmetic (protected access)
    REQUIRE(test_get_manual_phasecal() == 0);

    return 0;
}
