#ifndef MHO_EstimatePCManual_HH__
#define MHO_EstimatePCManual_HH__

#include <cctype>
#include <cmath>
#include <complex>
#include <map>
#include <vector>

#include "MHO_Constants.hh"
#include "MHO_Message.hh"

#include "MHO_ContainerDefinitions.hh"
#include "MHO_InspectingOperator.hh"
#include "MHO_ParameterStore.hh"
#include "MHO_TableContainer.hh"

namespace hops
{

/*!
 *@file MHO_EstimatePCManual.hh
 *@class MHO_EstimatePCManual
 *@author J. Barrett - barrettj@mit.edu
 *@date Wed May 29 03:40:50 PM EDT 2024
 *@brief
 */

/**
 * @brief Class MHO_EstimatePCManual
 */
class MHO_EstimatePCManual: public MHO_InspectingOperator< visibility_type >
{
    public:
        MHO_EstimatePCManual();
        virtual ~MHO_EstimatePCManual();

        /**
         * @brief Setter for weights
         * 
         * @param weights Input weights of type const weight_type*
         */
        void SetWeights(const weight_type* weights) { fWeights = weights; }

        /**
         * @brief Setter for phasors
         * 
         * @param phasors Input phasors of type phasor_type
         */
        void SetPhasors(phasor_type* phasors) { fPhasors = phasors; };

        /**
         * @brief Setter for parameter store
         * 
         * @param paramStore Pointer to MHO_ParameterStore object
         */
        void SetParameterStore(MHO_ParameterStore* paramStore) { fParameterStore = paramStore; };

        /**
         * @brief Setter for plot data
         * 
         * @param plot_data Reference to an mho_json object containing plot data.
         */
        void SetPlotData(mho_json& plot_data) { fPlotData.FillData(plot_data); }

    protected:
        /**
         * @brief Initializes implementation using input visibility type.
         * 
         * @param in Input visibility_type pointer for initialization
         * @return Boolean indicating successful initialization
         * @note This is a virtual function.
         */
        virtual bool InitializeImpl(const visibility_type* in) override { return true; };

        /**
         * @brief Executes manual PC estimation using input visibility data and retrieves mode logic.
         * 
         * @param in Input visibility_type pointer containing visibility data.
         * @return Always returns true.
         * @note This is a virtual function.
         */
        virtual bool ExecuteImpl(const visibility_type* in) override;

    private:
        /**
         * @brief Calculates and returns manual phase calibration for a given channel and polarization.
         * 
         * @param is_remote Flag indicating whether to use remote reference (true) or local reference (false).
         * @param channel_idx Index of the channel for which to calculate the phase calibration.
         * @param pol Polarization string ('L' or 'R')
         * @return Calculated manual phase calibration in degrees.
         */
        double get_manual_phasecal(int is_remote, int channel_idx, std::string pol);
        /**
         * @brief Calculates and returns manual delay offset for a given channel and polarization.
         * 
         * @param is_remote Flag indicating if the delay is remote (true) or local (false).
         * @param channel_idx Index of the channel, with 0 being the lowest frequency channel.
         * @param pol Polarization string ('L' or 'R').
         * @return Manual delay offset as a double.
         */
        double get_manual_delayoff(int is_remote, int channel_idx, std::string pol);

        MHO_ParameterStore fPlotData;
        MHO_ParameterStore* fParameterStore;
        phasor_type* fPhasors;
        const weight_type* fWeights;
        const visibility_type* fVisibilities;

        /**
         * @brief Manually estimates phase center using given mode and parameters.
         * 
         * @param mode Mode for estimating phase center.
         */
        void est_pc_manual(int mode);
        /**
         * @brief Estimates phases for reference or remote station with optional phase bias and displays messages.
         * 
         * @param is_ref Flag indicating whether to estimate phases on reference (true) or remote station (false)
         * @param keep Flag indicating whether to keep phase bias
         */
        void est_phases(int is_ref, int keep);

        /**
         * @brief Adjusts delays for Single Band Delay (SBD) and Effective Single Band Delay (ESBD) arrays based on given parameters.
         * 
         * @param sbd_max Maximum delay value for SBD
         * @param sbd Array of Single Band Delay values
         * @param esd Array of Effective Single Band Delay values
         * @param delta_delay Delay adjustment factor
         * @param first Starting index for delay adjustment
         * @param final Ending index for delay adjustment
         * @param is_ref Reference flag for delay adjustment
         * @param how Method flags for delay adjustment
         */
        void adj_delays(double sbd_max, double* sbd, double* esd, double delta_delay, int first, int final, int is_ref,
                        int how);
        /**
         * @brief Calculates and adjusts delays for fringe estimation based on reference station and delay modes.
         * 
         * @param is_ref Flag indicating whether to use reference station (true) or remote station (false).
         * @param how Bitmask specifying delay calculation modes.
         */
        void est_delays(int is_ref, int how);
        /**
         * @brief Calculates and adjusts phase offset for manual fringe tracking.
         * 
         * @param is_ref Flag indicating whether to use reference or remote station.
         */
        void est_offset(int is_ref);
        void masthead(int mode, std::string root_file, int first_ch, int final_ch);

        void fill_sbd(std::vector< std::string >& ch_labels, std::vector< double >& sbd);

        std::string fRemStationMk4ID;
        std::string fRefStationMk4ID;
        std::string fRemStationPol;
        std::string fRefStationPol;

        // //minor helper function to make sure all strings are compared as upper-case only
        // void make_upper(std::string& s){ for(char& c : s){c = toupper(c); };

        std::map< std::string, int > fChannelLabel2Index;
        std::map< int, std::string > fIndex2ChannelLabel;
};

} // namespace hops

#endif /*! end of include guard: MHO_EstimatePCManual */
