#include "MHO_BasicFringeFitter.hh"

//snapshot utility lib
#include "MHO_Snapshot.hh"
//initialization
#include "MHO_OperatorBuilderManager.hh"
#include "MHO_ElementTypeCaster.hh"

//pybind11 stuff to interface with python
#ifdef USE_PYBIND11
    #include <pybind11/pybind11.h>
    #include <pybind11/embed.h>
    #include "pybind11_json/pybind11_json.hpp"
    namespace py = pybind11;
    namespace nl = nlohmann;
    using namespace pybind11::literals;
#endif


namespace hops 
{


MHO_BasicFringeFitter::MHO_BasicFringeFitter():MHO_FringeFitter(){};
MHO_BasicFringeFitter::~MHO_BasicFringeFitter(){};

//basic run scheme: configure, init, then while(!IsFinished() ){ pre-run, run, post-run }
void MHO_BasicFringeFitter::Configure()
{
    
}

void MHO_BasicFringeFitter::Initialize()
{
    
}

void MHO_BasicFringeFitter::PreRun()
{
    
}

void MHO_BasicFringeFitter::Run()
{
    
}

void MHO_BasicFringeFitter::PostRun()
{
    
}

bool MHO_BasicFringeFitter::IsFinished()
{
    
}

//helper functions -- TODO determine which (if any) of these should be made public
std::string 
MHO_BasicFringeFitter::leftpadzeros_integer(unsigned int n_places, int value)
{
    std::stringstream ss;
    ss << std::setw(n_places);
    ss << std::setfill('0');
    ss << value;
    return ss.str();
}

std::string 
MHO_BasicFringeFitter::make_legacy_datetime_format(legacy_hops_date ldate)
{
    //formats the time as HHMMSS.xx with no separators (except the '.' for the fractional second)
    int isec = (int) ldate.second;
    float fsec = ldate.second - isec;
    std::string dt;
    dt = leftpadzeros_integer(2, ldate.hour) + leftpadzeros_integer(2, ldate.minute) + leftpadzeros_integer(2, isec);
    int fsec_dummy = (int) (100*fsec);
    dt += "." + leftpadzeros_integer(2, fsec_dummy);
    return dt;
}


double 
MHO_BasicFringeFitter::calculate_snr(double effective_npol, double ap_period, double samp_period, double total_ap_frac, double amp)
{
    //Poor imitation of SNR -- needs corrections
    //some hardcoded values used right now
    #pragma message("TODO FIXME -- need to accommodate stations with non-2bit sampling")
    double amp_corr_factor = 1.0;
    double fact1 = 1.0; //more than 16 lags
    double fact2 = 0.881; //2bit x 2bit
    double fact3 = 0.970; //difx
    double whitneys = 1e4; //unit conversion to 'Whitneys'
    double inv_sigma = fact1 * fact2 * fact3 * std::sqrt(ap_period/samp_period);
    double snr = amp * inv_sigma *  sqrt(total_ap_frac * effective_npol)/(whitneys * amp_corr_factor);
    return snr;
}

double 
MHO_BasicFringeFitter::calculate_mbd_no_ion_error(double freq_spread, double snr)
{
    double mbd_err = (1.0 / (2.0 * M_PI * freq_spread * snr) );
    return mbd_err;
}


double 
MHO_BasicFringeFitter::calculate_sbd_error(double sbd_sep, double snr, double sbavg)
{
    /* get proper weighting for sbd error estimate */
    // status->sbavg = 0.0;
    // for (fr = 0; fr < pass->nfreq; fr++)
    // for (ap = pass->ap_off; ap < pass->ap_off + pass->num_ap; ap++)
    // status->sbavg += pass->pass_data[fr].data[ap].sband;
    // status->sbavg /= status->total_ap;

    double sbd_err = (std::sqrt(12.0) * sbd_sep * 4.0) / (2.0 * M_PI * snr * (2.0 - std::fabs(sbavg)) ) ;
    return sbd_err;
}


double 
MHO_BasicFringeFitter::calculate_drate_error_v1(double snr, double ref_freq, double total_nap, double ap_delta)
{
    //originally: temp = status->total_ap * param->acc_period / pass->channels;
    //but we don't need the number of channels due to the difference in the way
    //we count 'APs' vs original c-code.
    double temp = total_nap*ap_delta;
    double drate_error = std::sqrt(12.0) / ( 2.0 * M_PI * snr * ref_freq * temp) ;
    return drate_error;
}


double 
MHO_BasicFringeFitter::calculate_drate_error_v2(double snr, double ref_freq, double integration_time)
{
    double temp = integration_time;
    double drate_error = std::sqrt(12.0) / ( 2.0 * M_PI * snr * ref_freq * temp) ;
    return drate_error;
}

double 
MHO_BasicFringeFitter::calculate_pfd(double snr, double pts_searched)
{
    double a = 1.0 - std::exp(-1.0*(snr*snr)/ 2.0);
    double pfd =  1.0 - std::pow(a, pts_searched);
    if(pfd < 0.01)
    {
        pfd = pts_searched * std::exp(-1.0*(snr*snr)/ 2.0);
    }
    return pfd;
}

double 
MHO_BasicFringeFitter::calculate_phase_error(double sbavg, double snr)
{
    //no ionosphere
    double sband_err = std::sqrt (1.0 + 3.0 * (sbavg * sbavg)); //why?
    double phase_err = 180.0 * sband_err / (M_PI * snr);
    return phase_err;
}

double 
MHO_BasicFringeFitter::calculate_phase_delay_error(double sbavg, double snr, double ref_freq)
{
    //no ionosphere
    double sband_err = std::sqrt (1.0 + 3.0 * (sbavg * sbavg));
    double ph_delay_err = sband_err / (2.0 * M_PI * snr * ref_freq);
    return ph_delay_err;
}


std::string 
MHO_BasicFringeFitter::calculate_qf()
{
    //dummy
    return std::string("?");
}


//more helper functions
void 
MHO_BasicFringeFitter::configure_data_library(MHO_ContainerStore* store)
{
    //retrieve the (first) visibility and weight objects
    //(currently assuming there is only one object per type)
    visibility_store_type* vis_store_data = nullptr;
    weight_store_type* wt_store_data = nullptr;

    vis_store_data = store->GetObject<visibility_store_type>(0);
    wt_store_data = store->GetObject<weight_store_type>(0);
    
    if(vis_store_data == nullptr)
    {
        msg_fatal("initialization", "failed to read visibility data from the .cor file." <<eom);
        std::exit(1);
    }

    if(wt_store_data == nullptr)
    {
        msg_fatal("initialization", "failed to read weight data from the .cor file." <<eom);
        std::exit(1);
    }

    std::size_t n_vis = store->GetNObjects<visibility_store_type>();
    std::size_t n_wt = store->GetNObjects<weight_store_type>();

    if(n_vis != 1 || n_wt != 1)
    {
        msg_warn("initialization", "multiple visibility and/or weight types per-baseline not yet supported" << eom);
    }

    auto vis_store_uuid = vis_store_data->GetObjectUUID();
    auto wt_store_uuid = wt_store_data->GetObjectUUID();

    std::string vis_shortname = store->GetShortName(vis_store_uuid);
    std::string wt_shortname = store->GetShortName(wt_store_uuid);
    
    visibility_type* vis_data = new visibility_type();
    weight_type* wt_data = new weight_type();
    
    //assign the storage UUID's to their up-casted counter-parts 
    //we do this so we can associate them to the file objects (w.r.t to program output, error messages, etc.)
    vis_data->SetObjectUUID(vis_store_uuid);
    wt_data->SetObjectUUID(wt_store_uuid);

    MHO_ElementTypeCaster<visibility_store_type, visibility_type> up_caster;
    up_caster.SetArgs(vis_store_data, vis_data);
    up_caster.Initialize();
    up_caster.Execute();

    MHO_ElementTypeCaster< weight_store_type, weight_type> wt_up_caster;
    wt_up_caster.SetArgs(wt_store_data, wt_data);
    wt_up_caster.Initialize();
    wt_up_caster.Execute();

    //remove the original objects to save space
    store->DeleteObject(vis_store_data);
    store->DeleteObject(wt_store_data);

    //warn on non-standard shortnames
    if(vis_shortname != "vis"){msg_warn("initialization", "visibilities do not use canonical short name 'vis', but are called: ", vis_shortname << eom);}
    if(wt_shortname != "weights"){msg_warn("initialization", "weights do not use canonical short name 'weights', but are called: ", wt_shortname << eom);}

    //now shove the double precision data into the container store with the same shortname
    store->AddObject(vis_data);
    store->AddObject(wt_data);
    store->SetShortName(vis_data->GetObjectUUID(), vis_shortname);
    store->SetShortName(wt_data->GetObjectUUID(), wt_shortname);
}

void 
MHO_BasicFringeFitter::extract_vex_info(const mho_json& vexInfo, MHO_ParameterStore* paramStore)


void 
MHO_BasicFringeFitter::calculate_fringe_info(MHO_ContainerStore* conStore, MHO_ParameterStore* paramStore, const mho_json& vexInfo)


void 
MHO_BasicFringeFitter::fill_plot_data(MHO_ParameterStore* paramStore, mho_json& plot_dict)


void 
MHO_BasicFringeFitter::precalculate_quantities(MHO_ContainerStore* conStore, MHO_ParameterStore* paramStore)

void 
MHO_BasicFringeFitter::init_and_exec_operators(MHO_OperatorBuilderManager& build_manager, MHO_OperatorToolbox* opToolbox, const char* category)

int 
MHO_BasicFringeFitter::parse_command_line(int argc, char** argv, MHO_ParameterStore* paramStore)

void 
MHO_BasicFringeFitter::basic_fringe_search(MHO_ContainerStore* conStore, MHO_ParameterStore* paramStore)

mho_json 
MHO_BasicFringeFitter::construct_plot_data(MHO_ContainerStore* conStore, MHO_ParameterStore* paramStore, mho_json& vexInfo)

void 
MHO_BasicFringeFitter::set_default_parameters(MHO_ContainerStore* conStore, MHO_ParameterStore* paramStore)




}//end namespace
