#include "MHO_AdhocPhaseCorrectionBuilder.hh"
#include "MHO_AdhocPhaseCorrection.hh"

#include <memory>

#include "MHO_Constants.hh"

namespace hops
{

bool MHO_AdhocPhaseCorrectionBuilder::Build()
{
    if(!IsConfigurationOk())
        return false;

    msg_debug("initialization", "building adhoc_phase correction operator." << eom);

    std::string op_name = fAttributes["name"].get< std::string >();
    std::string op_category = "calibration";
    double priority = fFormat["priority"].get< double >();

    //figure out what algo we are going to use
    std::string algorithm_type = fAttributes["value"]["algorithm_type"].get< std::string >();

    AdhocPhaseMode mode = AdhocPhaseMode::NONE;
    if(algorithm_type == "sinewave")
        mode = AdhocPhaseMode::SINEWAVE;
    else if(algorithm_type == "polynomial")
        mode = AdhocPhaseMode::POLYNOMIAL;
    else if(algorithm_type == "file")
        mode = AdhocPhaseMode::PHYLE;
    else
    {
        msg_error("initialization", "adhoc_phase: unrecognized algorithm_type '"
                                        << algorithm_type << "'. Expected 'sinewave', 'polynomial', or 'file'." << eom);
        return false;
    }

    //grab the ref/rem station codes
    std::string ref_id = fParameterStore->GetAs< std::string >("/ref_station/site_id");
    std::string rem_id = fParameterStore->GetAs< std::string >("/rem_station/site_id");

    //need the visibility data
    visibility_type* vis_data = fContainerStore->GetObject< visibility_type >(std::string("vis"));
    if(vis_data == nullptr)
    {
        msg_error("initialization", "cannot construct MHO_AdhocPhaseCorrection without visibility data." << eom);
        return false;
    }

    //time to build the operator
    std::unique_ptr< MHO_AdhocPhaseCorrection > op(new MHO_AdhocPhaseCorrection());
    op->SetArgs(vis_data);
    op->SetMode(mode);

    //SINEWAVE and POLYNOMIAL: both use a common reference time (seconds from scan start??)
    //TODO -- figure out what the hops3 implementation convention uses
    if(mode == AdhocPhaseMode::SINEWAVE || mode == AdhocPhaseMode::POLYNOMIAL)
    {
        double tref = 0.0;
        if(fParameterStore->IsPresent("/control/config/adhoc_tref"))
        {
            fParameterStore->Get("/control/config/adhoc_tref", tref);
        }
        op->SetTRef(tref);
    }

    //SINEWAVE style phase correction: amplitude (degrees -> radians?) and period (seconds?)
    if(mode == AdhocPhaseMode::SINEWAVE)
    {
        double amp_deg = 0.0;
        if(fParameterStore->IsPresent("/control/config/adhoc_amp"))
        {
            fParameterStore->Get("/control/config/adhoc_amp", amp_deg);
        }
        op->SetAmplitude(amp_deg * MHO_Constants::deg_to_rad);

        double period = 1.0;
        if(fParameterStore->IsPresent("/control/config/adhoc_period"))
        {
            fParameterStore->Get("/control/config/adhoc_period", period);
        }
        op->SetPeriod(period);
    }

    // POLYNOMIAL: coefficients (degrees/s^n -> radians/s^n)...we hope...
    if(mode == AdhocPhaseMode::POLYNOMIAL)
    {
        //std::cout<<"going to set polynomial coeff for adhoc"<<std::endl;
        std::vector< double > poly_coeffs;
        if(fParameterStore->IsPresent("/control/config/adhoc_poly"))
        {
            poly_coeffs = fParameterStore->GetAs< std::vector< double > >("/control/config/adhoc_poly");
        }
        for(double& coeff : poly_coeffs)
        {
            //std::cout<<"coeff = "<<coeff<<std::endl;
            coeff *= MHO_Constants::deg_to_rad;
        }
        op->SetPolynomialCoeffs(poly_coeffs);
    }

    // PHYLE: per-station adhoc files and channel strings.
    // Don't bother looking up the generic paths first, a generic phase-correction file makes no sense
    // just look up station-specific paths to the file parametersr
    if(mode == AdhocPhaseMode::PHYLE)
    {
        // --- adhoc_files ---
        std::string ref_file_path = std::string("/control/station/") + ref_id + "/adhoc_file";
        std::string rem_file_path = std::string("/control/station/") + rem_id + "/adhoc_file";
        std::string ref_file = "";
        std::string rem_file = "";

        if(fParameterStore->IsPresent(ref_file_path))
        {
            fParameterStore->Get(ref_file_path, ref_file);
        }
        if(fParameterStore->IsPresent(rem_file_path))
        {
            fParameterStore->Get(rem_file_path, rem_file);
        }

        // --- adhoc_file_chans ---
        std::string ref_chans_path = std::string("/control/station/") + ref_id + "/adhoc_file_chans";
        std::string rem_chans_path = std::string("/control/station/") + rem_id + "/adhoc_file_chans";
        std::string ref_chans = "";
        std::string rem_chans = "";

        if(fParameterStore->IsPresent(ref_chans_path))
        {
            fParameterStore->Get(ref_chans_path, ref_chans);
        }
        if(fParameterStore->IsPresent(rem_chans_path))
        {
            fParameterStore->Get(rem_chans_path, rem_chans);
        }

        op->SetRefAdhocFile(ref_file, ref_chans);
        op->SetRemAdhocFile(rem_file, rem_chans);

        msg_debug("initialization", "adhoc_phase (file mode): ref_file='" << ref_file << "' chans='" << ref_chans
                                                                          << "'  rem_file='" << rem_file << "' chans='"
                                                                          << rem_chans << "'" << eom);
    }

    // stash our adhoc phase operator in the toolbox
    op->SetName(op_name);
    op->SetPriority(priority);

    msg_debug("initialization", "creating adhoc_phase operator: " << op_name << " mode = " << algorithm_type << eom);

    bool replace_duplicates = true;
    this->fOperatorToolbox->AddOperator(std::move(op), op_name, op_category, replace_duplicates);
    return true;
}

} // namespace hops
