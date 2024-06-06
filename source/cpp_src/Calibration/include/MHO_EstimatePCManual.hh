#ifndef MHO_EstimatePCManual_HH__
#define MHO_EstimatePCManual_HH__


#include <cmath>
#include <complex>
#include <vector>
#include <map>
#include <cctype>

#include "MHO_Message.hh"
#include "MHO_Constants.hh"

#include "MHO_ParameterStore.hh"
#include "MHO_TableContainer.hh"
#include "MHO_ContainerDefinitions.hh"
#include "MHO_InspectingOperator.hh"



namespace hops
{

/*!
*@file MHO_EstimatePCManual.hh
*@class MHO_EstimatePCManual
*@author J. Barrett - barrettj@mit.edu
*@date Wed May 29 03:40:50 PM EDT 2024
*@brief
*/


class MHO_EstimatePCManual: public MHO_InspectingOperator< visibility_type >
{
    public:

        MHO_EstimatePCManual();
        virtual ~MHO_EstimatePCManual();

        void SetWeights(const weight_type* weights){fWeights = weights;}
        void SetPhasors(phasor_type* phasors){fPhasors = phasors;};
        void SetParameterStore(MHO_ParameterStore* paramStore){fParameterStore = paramStore;}; // TODO replace me

        // void SetStation(std::string station){fStationCode = station;}; //2-char station code
        // void SetStationMk4ID(std::string station_id){fMk4ID = station_id;} //1-char mk4id
        // void SetPolarization(const std::string& pol){fPol = pol; make_upper(fPol);};
        // void SetPCPhaseOffset(const double& pc_phase_offset){fPhaseOffset = pc_phase_offset;}

        void SetPlotData(mho_json& plot_data)
        {
            fPlotData.FillData(plot_data);
        }

    protected:

        virtual bool InitializeImpl(const visibility_type* in) override { return true; };

        virtual bool ExecuteImpl(const visibility_type* in) override;

    private:

        double get_manual_phasecal(int is_remote, int channel_idx, std::string pol);

        MHO_ParameterStore fPlotData;
        MHO_ParameterStore* fParameterStore;
        phasor_type* fPhasors;
        const weight_type* fWeights;
        const visibility_type* fVisibilities;

        void est_pc_manual(int mode);
        void est_phases(int is_ref, int keep);

        void adj_delays(double sbd_max, double* sbd, double* esd, double delta_delay, int first, int final, int is_ref, int how);
        void est_delays(int is_ref, int how);
        void est_offset(int is_ref);
        void masthead(int mode, std::string root_file, int first_ch, int final_ch);
        
        void fill_sbd(std::vector<std::string>& ch_labels, std::vector<double>& sbd);

        // std::size_t DetermineStationIndex(const phasor_type* in);
        // bool PolMatch(std::size_t station_idx, std::string& polprod);
        //
        // //constants
        // std::complex<double> fImagUnit;
        // double fDegToRad;
        //
        // //selection
        // std::string fStationCode;
        // std::string fMk4ID;
        // std::string fPol;
        //
        // //pc rotation
        // double fPhaseOffset;
        //
        // //keys for tag retrieval
        // std::string fStationKey;
        // std::string fRemStationKey;
        // std::string fRefStationKey;

        std::string fRemStationMk4ID;
        std::string fRefStationMk4ID;
        std::string fRemStationPol;
        std::string fRefStationPol;

        // std::string fChannelLabelKey;

        // //minor helper function to make sure all strings are compared as upper-case only
        // void make_upper(std::string& s){ for(char& c : s){c = toupper(c); };

};


}


#endif /*! end of include guard: MHO_EstimatePCManual */
