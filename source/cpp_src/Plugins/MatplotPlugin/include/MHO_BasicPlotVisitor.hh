#ifndef MHO_BasicPlotVisitor_HH__
#define MHO_BasicPlotVisitor_HH__

#include "MHO_FringePlotVisitor.hh"
#include "MHO_JSONHeaderWrapper.hh"
#include "MHO_PlotDataExtractor.hh"

#include <algorithm>
#include <cmath>
#include <iomanip>
#include <matplot/matplot.h>
#include <numeric>
#include <sstream>
#include <string>
#include <vector>
#include <utility>

namespace hops
{

class MHO_BasicPlotVisitor: public MHO_FringePlotVisitor
{
    public:
        MHO_BasicPlotVisitor();
        virtual ~MHO_BasicPlotVisitor();

    protected:
        /**
         * @brief Main plotting function - creates the complete fourfit plot
         * @param data Input MHO_FringeData for plotting
         */
        virtual void Plot(MHO_FringeData* data) override;

    private:


        class subplot_parameters
        {
            public:
                subplot_parameters():
                    total_rows(0),
                    total_cols(0),
                    start_row(0),
                    start_col(0),
                    rowspan(0),
                    colspan(0)
                {};

                subplot_parameters(int a, int b, int c, int d, int e, int f):
                    total_rows(a),
                    total_cols(b),
                    start_row(c),
                    start_col(d),
                    rowspan(e),
                    colspan(f)
                {};

                virtual ~subplot_parameters(){};

            //data
                int total_rows;
                int total_cols;
                int start_row;
                int start_col;
                int rowspan;
                int colspan;
        };

        std::map<std::string, subplot_parameters> fSubplotConfig;

        void ConfigureSubplots();

        void ConstructXTitle(const subplot_parameters& params, std::string title, std::string font_color, int font_size, double x_coord, double y_coord);
        void ConstructYTitle();

        void ConstructPlot(const mho_json& plot_data);

        void DirectSavePlot(std::string filename);

        /**
         * @brief Creates the delay rate and multiband delay twin plot
         * @param plot_dict JSON dictionary containing plot data
         */
        void make_dr_mbd_plot(const mho_json& plot_dict);

        /**
         * @brief Creates the single-band delay and ionospheric dTEC twin plot
         * @param plot_dict JSON dictionary containing plot data
         */
        void make_sbd_dtec_plot(const mho_json& plot_dict);

        /**
         * @brief Creates the cross-power spectrum amplitude/phase twin plot
         * @param plot_dict JSON dictionary containing plot data
         */
        void make_xpower_plot(const mho_json& plot_dict);

        /**
         * @brief Creates per-channel amplitude and phase segment plots
         * @param plot_dict JSON dictionary containing plot data
         */
        void make_channel_segment_plots(const mho_json& plot_dict);

        /**
         * @brief Creates USB/LSB validity flag indicator plots between segments and pcal
         * @param plot_dict JSON dictionary containing plot data
         */
        void make_channel_segment_validity_plots(const mho_json& plot_dict);

        /**
         * @brief Creates per-channel phase calibration plots
         * @param plot_dict JSON dictionary containing plot data
         */
        void make_pcal_plots(const mho_json& plot_dict);

        /**
         * @brief Adds basic title and identification text
         * @param plot_dict JSON dictionary containing plot data
         */
        void make_top_info_text(const mho_json& plot_dict);

        /**
         * @brief Adds basic fringe summary information
         * @param plot_dict JSON dictionary containing plot data
         */
        void make_basic_info_text(const mho_json& plot_dict);

        /**
         * @brief Adds a priori model, totals, and residuals text at bottom
         * @param plot_dict JSON dictionary containing plot data
         */
        void make_model_resid_info_text(const mho_json& plot_dict);

        /**
         * @brief Adds fringe RMS table
         * @param plot_dict JSON dictionary containing plot data
         */
        void make_rms_table(const mho_json& plot_dict);

        /**
         * @brief Adds station coordinate statements (az,el,pa,u,v)
         * @param plot_dict JSON dictionary containing plot data
         */
        void make_coord_text(const mho_json& plot_dict);

        /**
         * @brief Adds amplitude table
         * @param plot_dict JSON dictionary containing plot data
         */
        void make_amplitude_table(const mho_json& plot_dict);

        /**
         * @brief Adds window limits table (sbd,mbd,dr,ion)
         * @param plot_dict JSON dictionary containing plot data
         */
        void make_window_table(const mho_json& plot_dict);

        /**
         * @brief Adds data statistics/summary text
         * @param plot_dict JSON dictionary containing plot data
         */
        void make_data_stats_text(const mho_json& plot_dict);

        /**
         * @brief Creates the channel information table below pcal plots
         * @param plot_dict JSON dictionary containing plot data
         */
        void make_channel_info_table(const mho_json& plot_dict);

        /**
         * @brief Sets up the overall figure layout and spacing
         */
        void setup_figure_layout();


        double determine_desired_tick_spacing(int n_ticks, double lower_limit, double upper_limit, const std::vector<double>& desired_intervals)
        {
                // Calculate tick marks based on span and desired spacing intervals
                double span = std::fabs(upper_limit - lower_limit);
                double target = span/(double)n_ticks;
                //example target spacing:
                //std::vector<double> tspace = {0.1, 0.5, 1, 5, 10, 25, 50, 100, 200, 500};
                int idx = 0;
                double tmin = 1e3*upper_limit;
                for(std::size_t i=0; i<desired_intervals.size(); i++)
                {
                    double delta = std::fabs( target - desired_intervals[i] );
                    if(delta < tmin){tmin = delta; idx = i;}
                }
                double tick_spacing = desired_intervals[idx];
                return tick_spacing;
        }

        /**
         * @brief Custom implementation of matplotlib's subplot2grid functionality
         * @param shape Grid shape as {rows, cols}
         * @param loc Starting position as {row, col}
         * @param rowspan Number of rows to span (default 1)
         * @param colspan Number of columns to span (default 1)
         * @return axes_handle for the created subplot
         */
        matplot::axes_handle subplot2grid(const std::pair< int, int >& shape, const std::pair< int, int >& loc, int rowspan = 1,
                                          int colspan = 1);

        matplot::axes_handle subplot2grid_wrapper(const subplot_parameters sp);

        /**
         * @brief Specialized subplot function for channel plots with equal spacing
         * @param total_rows Total number of rows in grid
         * @param start_row Starting row position
         * @param row_height Number of rows to span
         * @param channel_index Which channel (0-based)
         * @param total_channels Total number of channels
         * @return axes_handle for the created subplot
         */
        matplot::axes_handle channel_subplot(int total_rows, int start_row, int row_height, int channel_index,
                                             int total_channels);

        // Member variables for plot state
        matplot::figure_handle fCurrentFigure;
        bool fShowPlot;
        std::string fFilename;
        mho_json fPlotData;

        //place to stash all the axes so they stay part of the figure
        std::vector<matplot::axes_handle> fAxes;
        matplot::axes_handle fLastAxis;
        
        int fPageWidth;
        int fPageHeight;
        
};

} // namespace hops

#endif /* end of include guard: MHO_BasicPlotVisitor_HH__ */
