#include "MHO_BasicPlotVisitor.hh"
#include "MHO_Message.hh"
#include "MHO_Tokenizer.hh"
#include "MHO_DirectoryInterface.hh"

#include <iomanip>
#include <sstream>
#include <cctype>
#include <cstdlib>
#include <termios.h> // for file name capture on command line


std::string extension_to_terminal_cmd(const std::string& ext) 
{
    //we only support .pdf, .eps, and .svg output
    if (ext == "pdf")  return "set term pdfcairo size 8.5in,11in fontscale 0.6";
    if (ext == "svg")  return "svg size 816,1056 dynamic fontscale 0.5";
    if (ext == "eps")  return "set term postscript portrait size 8.5in,11in fontscale 0.8";
    return "";
}


namespace hops
{

MHO_BasicPlotVisitor::MHO_BasicPlotVisitor(): 
    fShowPlot(true), 
    fFilename("")
{
    //standard 8.5"x11" letter size (100 DPI)
    fPageWidth = 850;
    fPageHeight = 1100;

    //A4 size 
    //fPageWidth = 827;
    //fPageHeight = 1169;
}

MHO_BasicPlotVisitor::~MHO_BasicPlotVisitor(){}

void MHO_BasicPlotVisitor::ConstructPlot(const mho_json& fPlotData)
{

    ConfigureSubplots();
    //add plots
    make_dr_mbd_plot(fPlotData);
    make_sbd_dtec_plot(fPlotData);
    make_xpower_plot(fPlotData);
    make_channel_segment_plots(fPlotData);
    make_channel_segment_validity_plots(fPlotData);
    make_pcal_plots(fPlotData);
    //add text
    make_top_info_text(fPlotData);
    make_basic_info_text(fPlotData);
    make_model_resid_info_text(fPlotData);
    make_rms_table(fPlotData);
    make_coord_text(fPlotData);
    make_amplitude_table(fPlotData);
    make_window_table(fPlotData);
    make_data_stats_text(fPlotData);
    make_channel_info_table(fPlotData);
}

void MHO_BasicPlotVisitor::ConfigureSubplots()
{
    //basis for layout
    int nrows = 35;
    int ncols = 64;
    fNRows = nrows;
    fNCols = ncols;

    fSubplotConfig["mbd_plot"] = subplot_parameters(2*nrows, ncols, 3, 3, 12, 46);
    fSubplotConfig["mbd_title"] = subplot_parameters(2*nrows, ncols, 2, 2, 1, 48);
    fSubplotConfig["mbd_amp_ytitle"] = subplot_parameters(2*nrows, 2*ncols, 5, 4, 8, 1);
    fSubplotConfig["delay_rate_xtitle"] = subplot_parameters(2*nrows, ncols, 2, 2, 15, 48);
    fSubplotConfig["sbd_plot"] = subplot_parameters(2*nrows, ncols, 19, 3, 8, 21);
    fSubplotConfig["sbd_amp_ytitle"] = subplot_parameters(2*nrows, 2*ncols, 19, 4, 8, 1);
    fSubplotConfig["ion_tec_title"] = subplot_parameters(2*nrows, ncols, 18, 4, 1, 14);
    fSubplotConfig["sbd_title"] = subplot_parameters(2*nrows, ncols, 28, 2, 1, 14);
    fSubplotConfig["xpower_plot"] = subplot_parameters(2*nrows, 2*ncols, 19, 56, 8, 42);
    fSubplotConfig["xpower_xtitle"] = subplot_parameters(2*nrows, ncols, 28, 32, 1, 2);
    fSubplotConfig["xpower_phase_ytitle"] = subplot_parameters(2*nrows, ncols, 19, 49, 8, 2);
    fSubplotConfig["channel_phase_ytitle"] = subplot_parameters(2*nrows, ncols, 32, 60, 8, 1);
    fSubplotConfig["channel_amp_ytitle"] = subplot_parameters(2*nrows, 2*ncols, 32, 4, 8, 1);
    fSubplotConfig["pcal_theta_ytitle"] = subplot_parameters(2*nrows, 2*ncols, 40, 119, 8, 1);
    fSubplotConfig["station_codes"] = subplot_parameters(4*nrows, 2*ncols, 87, 119, 2, 1);
    fSubplotConfig["usblsb_frac"] = subplot_parameters(4*nrows, 4*ncols, 81, 9, 2, 1);
    fSubplotConfig["top_info_textbox"] = subplot_parameters(nrows, ncols, 0, 0, 2, 62);
    fSubplotConfig["basic_info_textbox"] = subplot_parameters(nrows, ncols, 2, 52, 18, 12);
    fSubplotConfig["model_resid_info_textbox"] = subplot_parameters(2*nrows, ncols, 57, 0, 10, 64);
    fSubplotConfig["rms_textbox"] = subplot_parameters(nrows, ncols, 31, 0, 4, 16);
    fSubplotConfig["coord_textbox"] = subplot_parameters(nrows, ncols, 33, 0, 2, 64);
    fSubplotConfig["amp_table_textbox"] = subplot_parameters(nrows, ncols, 31, 12, 4, 13);
    fSubplotConfig["window_textbox"] = subplot_parameters(nrows, ncols, 31, 49, 4, 12);
    fSubplotConfig["stats_textbox"] = subplot_parameters(nrows, ncols, 31, 29, 4, 17);

    fLeftMargin = 0.045;
    fRightMargin = 0.08;
}


void MHO_BasicPlotVisitor::ConstructXTitle(const subplot_parameters& params, std::string title, std::string font_color, int font_size, double x_coord, double y_coord, bool center)
{
    try
    {
        //create a small text-only subplot
        auto text_ax = subplot2grid_wrapper(params);

        //turn off axis display for text subplot
        text_ax->x_axis().visible(false);
        text_ax->y_axis().visible(false);
        text_ax->box(false);
        
        // Set up coordinate system for text placement (0-1 range)
        text_ax->xlim({0, 1});
        text_ax->ylim({0, 1});
        
        // Add centered axis label
        auto label = text_ax->text(x_coord, y_coord, title);
        label->color(font_color);
        label->font_size(font_size);
        if(center){label->alignment(matplot::labels::alignment::center);}
        
        msg_debug("plot", "added title: "<<title<<" to plot" << eom);
    }
    catch(const std::exception& e)
    {
        msg_warn("plot", "failed to add title: "<<title<<" to plot: " << e.what() << eom);
    }

}


void MHO_BasicPlotVisitor::ConstructYTitle(const subplot_parameters& params, std::string title, std::string font_color, int font_size, bool is_y2)
{
    try
    {
        // Create a small text-only subplot for the y-axis label
        auto text_ax = subplot2grid_wrapper(params);

        // Turn off axis display for text subplot
        if(!is_y2)
        {
            text_ax->x_axis().visible(false);
            text_ax->y_axis().visible(false);
            text_ax->y2_axis().visible(false);
            text_ax->box(false);

            text_ax->font_size(font_size);
            text_ax->y_axis().label_font_size(8);
            text_ax->y_axis().label(title);
            text_ax->y_axis().label_color(font_color);
            text_ax->y_axis().tick_values({});
        }
        else 
        {
            text_ax->x_axis().visible(false);
            text_ax->y_axis().visible(false);
            text_ax->y2_axis().visible(false);
            text_ax->box(false);

            text_ax->font_size(font_size);
            text_ax->y2_axis().label_font_size(8);
            text_ax->y2_axis().label(title);
            text_ax->y2_axis().label_color(font_color);
            text_ax->y2_axis().tick_values({});
        }

        msg_debug("plot", "adding y-axis title: "<< title << " to plot" << eom);
    }
    catch(const std::exception& e)
    {
        msg_warn("plot", "failed to add y-axis title: "<< title << " to plot: " << e.what() << eom);
    }
}

void MHO_BasicPlotVisitor::DirectSavePlot(std::string filename)
{
    //matplot++ needs special backend treatment when calling .save()
    auto backend = fCurrentFigure->backend();
    std::string ext = MHO_DirectoryInterface::GetFileExtension(filename);
    std::string cmd = extension_to_terminal_cmd(ext);
    msg_debug("fringe", "plotting backend setup command: "<< cmd << eom);
    if(cmd == "")
    {
        msg_warn("fringe", "only .pdf, .eps, and .svg export is available from the command line, " << 
                 "use the plot GUI to export to other formats" << eom );
        return;
    }
    backend->run_command(cmd);
    ConstructPlot(fPlotData);
    fCurrentFigure->save(filename);
    msg_info("fringe", "plot saved to: " << filename << eom);
}

void MHO_BasicPlotVisitor::Plot(MHO_FringeData* data)
{
    bool is_skipped = data->GetParameterStore()->GetAs< bool >("/status/skipped");
    if(is_skipped)
    {
        msg_debug("plot", "plotting disabled or scan skipped, returning" << eom);
        return;
    }

    //check if we should plot
    fShowPlot = data->GetParameterStore()->GetAs< bool >("/cmdline/show_plot");
    // Get output filename if specified
    fFilename = "";
    if(data->GetParameterStore()->IsPresent("/cmdline/disk_file"))
    {
        fFilename = data->GetParameterStore()->GetAs< std::string >("/cmdline/disk_file");
    }

    try
    {
        msg_debug("fringe", "attempting to plot data with matplot++/gnuplot plotting utility" << eom);
        setup_figure_layout();
        fPlotData = data->GetPlotData();

        if(!fFilename.empty())
        {
            msg_debug("plot", "saving plot to: " << fFilename << eom);
            DirectSavePlot(fFilename);
        }
        else 
        {
            // build the plot from fringe data
            ConstructPlot(fPlotData);
        }

        if(fShowPlot)
        {
            msg_debug("plot", "displaying plot" << eom);
            fCurrentFigure->draw(); // don't use show();
            bool loop = true;
            msg_info("fringe", "press a key: 's'=save (as .pdf, .eps, or .svg), 'q'=quit, other=continue" << eom);
            while(loop) 
            {
                // Save old terminal settings
                termios oldt;
                tcgetattr(STDIN_FILENO, &oldt);

                // Set new terminal settings
                termios newt = oldt;
                newt.c_lflag &= ~(ICANON | ECHO); // Turn off canonical mode & echo
                newt.c_cc[VMIN] = 1;              // Read returns after 1 byte
                newt.c_cc[VTIME] = 0;             // No timeout
                tcsetattr(STDIN_FILENO, TCSANOW, &newt);

                //get 1 character
                char c = getchar();

                // Restore old settings
                tcsetattr(STDIN_FILENO, TCSANOW, &oldt);

                switch (c) 
                {
                    case 's':
                    case 'S': 
                    {
                        msg_info("fringe", "type the filename to save the plot to: " << eom);
                        std::cout.flush();
                        std::string response;
                        std::getline(std::cin, response);
                        std::string fileName = MHO_Tokenizer::TrimLeadingAndTrailingWhitespace(response);
                        DirectSavePlot(fileName);
                        break;
                    }
                    case 'q':
                        std::exit(1);
                    default:
                        msg_info("fringe", "continuing..." << eom);
                        loop = false;
                        break;
                }
            }
        }
    }
    catch(const std::exception& e)
    {
        msg_error("plot", "exception during plotting: " << e.what() << eom);
    }
}

void MHO_BasicPlotVisitor::setup_figure_layout()
{
    // create figure with appropriate size (8.5 x 11 inches equivalent)
    // TODO FIXME -- add support for A4 size paper (this may require tweaking plot/text positioning)
    fCurrentFigure = matplot::figure(true); //must be true (quiet mode)!
    fCurrentFigure->size(fPageWidth, fPageHeight);
    fCurrentFigure->position({100, 100});
    // Set white background to fix grey background issue
    fCurrentFigure->color("white");
}


matplot::axes_handle MHO_BasicPlotVisitor::subplot2grid_wrapper(const subplot_parameters sp, double left_margin, double right_margin)
{
    return subplot2grid({sp.total_rows, sp.total_cols}, {sp.start_row, sp.start_col}, sp.rowspan, sp.colspan, left_margin, right_margin);
}


matplot::axes_handle MHO_BasicPlotVisitor::subplot2grid(const std::pair< int, int >& shape, 
                                                        const std::pair< int, int >& loc,
                                                        int rowspan, int colspan, float left_margin, float right_margin)
{
    int total_rows = shape.first;
    int total_cols = shape.second;
    int start_row = loc.first;
    int start_col = loc.second;

    float left = static_cast< float >(start_col) / total_cols;
    float width = static_cast< float  >(colspan) / total_cols;

    //calculate normalized position and size
    if(left_margin != 0.0 || right_margin != 0.0)
    {
        width = (static_cast< float >(colspan) - (right_margin+left_margin) ) / total_cols;
        left = start_col*width + left_margin;
    }

    // Flip row calculation since matplotlib uses bottom-origin, we use top-origin
    float bottom = static_cast< float  >(total_rows - start_row - rowspan) / total_rows;
    float height = static_cast< float  >(rowspan) / total_rows;

    // if we want space between plots, change value of 'padding' below to something non-zero
    const double padding = 0.0;
    left += padding / 2;
    bottom += padding / 2;
    width -= padding;
    height -= padding;

    fAxes.push_back( fCurrentFigure->add_axes( {left, bottom, width, height}) );
    fLastAxis = fAxes.back();
    return fLastAxis;
}

void MHO_BasicPlotVisitor::make_dr_mbd_plot(const mho_json& plot_dict)
{
    msg_debug("plot", "creating delay rate / multiband delay plot" << eom);

    // Extract data for delay rate
    auto dlyrate = MHO_PlotDataExtractor::extract_vector(plot_dict, "DLYRATE");
    auto dly_x = MHO_PlotDataExtractor::extract_vector(plot_dict, "DLYRATE_XAXIS");
    if(dly_x.empty() && !dlyrate.empty())
    {
        dly_x = MHO_PlotDataExtractor::create_index_vector(dlyrate.size());
    }

    // Extract data for multiband delay
    auto mbd_amp = MHO_PlotDataExtractor::extract_vector(plot_dict, "MBD_AMP");
    auto mbd_x = MHO_PlotDataExtractor::extract_vector(plot_dict, "MBD_AMP_XAXIS");
    if(mbd_x.empty() && !mbd_amp.empty())
    {
        mbd_x = MHO_PlotDataExtractor::create_index_vector(mbd_amp.size());
    }

    if(dlyrate.empty() && mbd_amp.empty())
    {
        msg_warn("plot", "No delay rate or MBD data available" << eom);
        return;
    }

    auto ax_dr = subplot2grid_wrapper(fSubplotConfig["mbd_plot"]);

    ax_dr->font_size(8);
    fLastAxis->hold(matplot::on);

    // Determine the common x-axis range - use delay rate range as primary
    double x_min = 0.0, x_max = 1.0;
    if(!dly_x.empty())
    {
        x_min = dly_x.front();
        x_max = dly_x.back();
    }
    else if(!mbd_x.empty())
    {
        x_min = mbd_x.front();
        x_max = mbd_x.back();
    }

    // Capture the proper y-limits before any auto-expansion occurs
    std::array< double, 2 > proper_y_limits;

    // Plot delay rate data (red) - use original x-axis
    if(!dlyrate.empty() && !dly_x.empty())
    {
        auto line1 = matplot::plot(dly_x, dlyrate, "r-");
        line1->line_width(0.5f);

        // Configure axis properties - enable minor grid and rotate y-tick labels
        auto ax_handle = matplot::gca();
        ax_handle->minor_grid(true); // Enable minor grid lines as substitute for minor ticks
        // Capture y-limits immediately after first plot, before auto-expansion
        proper_y_limits = matplot::ylim();
        msg_debug("plot", "Captured original Y limits: " << proper_y_limits[0] << " to " << proper_y_limits[1] << eom);
    }

    // Plot MBD data (blue) - rescale x-axis to match delay rate range
    if(!mbd_amp.empty() && !mbd_x.empty())
    {
        // Rescale MBD x-axis to span the same range as delay rate
        std::vector< double > mbd_x_rescaled(mbd_x.size());

        if(mbd_x.size() > 1)
        {
            double mbd_x_range = mbd_x.back() - mbd_x.front();
            double target_range = x_max - x_min;

            for(size_t i = 0; i < mbd_x.size(); ++i)
            {
                // Normalize to 0-1, then scale to target range
                double normalized = (mbd_x[i] - mbd_x.front()) / mbd_x_range;
                mbd_x_rescaled[i] = x_min + normalized * target_range;
            }
        }
        else
        {
            // Single point case - unlikely
            mbd_x_rescaled[0] = (x_min + x_max) / 2.0;
        }

        auto line2 = matplot::plot(mbd_x_rescaled, mbd_amp, "b-");
        line2->line_width(0.5f);
        line2->use_y2(true);

        // Force both y-axes to use the original data limits (no auto-expansion)
        if(proper_y_limits.size() >= 2)
        {
            try
            {
                auto current_ax = matplot::gca();
                if(current_ax)
                {
                    // Set both axes to use the original data limits
                    matplot::ylim({proper_y_limits[0], proper_y_limits[1]});
                    current_ax->y_axis().limits_mode_auto(false);

                    current_ax->y2_axis().limits({proper_y_limits[0], proper_y_limits[1]});
                    current_ax->y2_axis().limits_mode_auto(false);

                    msg_debug("plot", "Set both Y axes to original limits: " << proper_y_limits[0] << " to "
                                                                             << proper_y_limits[1] << eom);
                }
            }
            catch(const std::exception& e)
            {
                msg_warn("plot", "Failed to synchronize y-axes: " << e.what() << eom);
            }
        }

        // Add twin x-axis labels within plot bounds
        try
        {
            // Get the plot area bounds after fixing limits
            auto y_lim = matplot::ylim();
            auto x_lim = matplot::xlim();

            if(y_lim.size() >= 2 && x_lim.size() >= 2)
            {
                double y_top = y_lim[1];
                double y_bottom = y_lim[0];
                double y_range = y_top - y_bottom;
                double x_left = x_lim[0];
                double x_right = x_lim[1];
                double x_range = x_right - x_left;

                // Position elements at the very top edge (now that limits are fixed)
                double tick_mark_y = y_top - 0.03 * y_range;  // Tick marks extend 3% down from top edge (bigger)
                double tick_label_y = y_top - 0.05 * y_range; // Labels 5% down from top edge
                double axis_label_y = y_top - 0.08 * y_range; // Axis label 8% down from top edge

                // Make tick mark spacing adjustable (in original MBD units)
                // Calculate tick marks based on spacing interval
                double mbd_min = mbd_x.front();
                double mbd_max = mbd_x.back();
                double orig_mbd_range = mbd_max - mbd_min;
                std::vector<double> tspace = {0.0001, 0.0005, 0.001, 0.005, 0.01, 0.05, 0.1, 0.5, 1., 5., 10., 50., 100., 500, 1000.};
                double tick_spacing = determine_desired_tick_spacing(7, mbd_min, mbd_max, tspace);

                std::vector< double > tick_x_positions;
                std::vector< double > tick_values;
                // Find the first tick mark (round down to nearest spacing interval)
                double first_tick = std::floor(mbd_min / tick_spacing) * tick_spacing;

                // Generate tick marks at regular intervals
                for(double tick_val = first_tick; tick_val <= mbd_max + tick_spacing * 0.001; tick_val += tick_spacing)
                {
                    // Only include ticks within the data range
                    if(tick_val >= mbd_min && tick_val <= mbd_max)
                    {
                        // Calculate position on rescaled axis
                        double t = (tick_val - mbd_min) / orig_mbd_range;

                        // Map to rescaled x-coordinate with small inset for visibility
                        double mbd_range = mbd_x_rescaled.back() - mbd_x_rescaled.front();
                        double inset = mbd_range * 0.01; // 1% inset from edges
                        double mbd_left = mbd_x_rescaled.front() + inset;
                        double mbd_right = mbd_x_rescaled.back() - inset;
                        double mbd_span = mbd_right - mbd_left;

                        double tick_pos = mbd_left + t * mbd_span;
                        tick_x_positions.push_back(tick_pos);
                        tick_values.push_back(tick_val);
                    }
                }

                // Draw tick marks and labels
                for(size_t i = 0; i < tick_x_positions.size() && i < tick_values.size(); ++i)
                {
                    double tick_x = tick_x_positions[i];
                    double tick_val = tick_values[i];

                    // Draw tick mark (bigger vertical line)
                    std::vector< double > tick_line_x = {tick_x, tick_x};
                    std::vector< double > tick_line_y = {y_top, tick_mark_y};
                    auto tick_mark = matplot::plot(tick_line_x, tick_line_y, "-");
                    tick_mark->color("blue");
                    tick_mark->line_width(1.0f); // Thicker line

                    // Add tick label centered on tick mark
                    std::ostringstream tick_stream;
                    tick_stream << std::fixed << std::setprecision(3) << tick_val;
                    // auto tick_text = fLastAxis->text(tick_x, tick_label_y, tick_stream.str());
                    auto tick_text = fLastAxis->text(tick_x, tick_label_y, tick_stream.str());
                    tick_text->color("blue");
                    tick_text->font_size(7);
                    tick_text->alignment(matplot::labels::alignment::center);
                }

                // Skip axis label inside plot - will add it in separate text area

                msg_debug("plot",
                          "Added twin x-axis with MBD range: " << mbd_x.front() << " to " << mbd_x.back() << " us" << eom);
            }
        }
        catch(const std::exception& e)
        {
            msg_warn("plot", "Failed to add twin x-axis labels: " << e.what() << eom);
        }
    }

    // Set the final x-axis limits
    matplot::xlim({x_min, x_max});

    // Add twin x-axis label in separate text area above the plot (safe approach)
    if(!mbd_x.empty())
    {
        ConstructXTitle(fSubplotConfig["mbd_title"], "multiband delay ({/Symbol m})", "blue", 9, 0.5, 0.5, true);
    }

    ConstructYTitle(fSubplotConfig["mbd_amp_ytitle"], "amplitude", "red", 8);
    ConstructXTitle(fSubplotConfig["delay_rate_xtitle"], "delay rate (ns/s)", "red", 9, 0.5, 0.0, true);

}

void MHO_BasicPlotVisitor::make_sbd_dtec_plot(const mho_json& plot_dict)
{
    msg_debug("plot", "Creating single-band delay / dTEC plot" << eom);

    // Extract single-band delay data
    auto sbd_amp = MHO_PlotDataExtractor::extract_vector(plot_dict, "SBD_AMP");
    auto sbd_x = MHO_PlotDataExtractor::extract_vector(plot_dict, "SBD_AMP_XAXIS");
    if(sbd_x.empty() && !sbd_amp.empty())
    {
        sbd_x = MHO_PlotDataExtractor::create_index_vector(sbd_amp.size());
    }

    if(sbd_amp.empty())
    {
        msg_warn("plot", "No SBD data available" << eom);
        return;
    }
    auto ax = subplot2grid_wrapper(fSubplotConfig["sbd_plot"]);
    ax->font_size(8);

    // Set up the plotting area
    fLastAxis->hold(matplot::on);

    // Determine the common x-axis range - use SBD range as primary
    double x_min = 0.0, x_max = 1.0;
    if(!sbd_x.empty())
    {
        x_min = sbd_x.front();
        x_max = sbd_x.back();
    }

    // Determine y-axis limits by combining both SBD and dTEC data
    double y_min = 0.0, y_max = 1.0;
    if(!sbd_amp.empty())
    {
        auto sbd_minmax = std::minmax_element(sbd_amp.begin(), sbd_amp.end());
        y_min = std::min(y_min, *sbd_minmax.first);
        y_max = std::max(y_max, *sbd_minmax.second);
    }

    // Check for dTEC data and include in y-axis scaling
    std::vector< double > dtec_y;
    std::vector< double > dtec_x_rescaled;
    std::vector< double > dtec_x; // Keep original dTEC x values for twin axis
    if(plot_dict.contains("extra") && plot_dict["extra"].contains("dtec_array"))
    {
        try
        {
            dtec_x = plot_dict["extra"]["dtec_array"].get< std::vector< double > >();
            dtec_y = plot_dict["extra"]["dtec_amp_array"].get< std::vector< double > >();

            if(!dtec_x.empty() && !dtec_y.empty())
            {
                // Include dTEC data in y-axis scaling
                auto dtec_minmax = std::minmax_element(dtec_y.begin(), dtec_y.end());
                y_min = std::min(y_min, *dtec_minmax.first);
                y_max = std::max(y_max, *dtec_minmax.second);

                // Rescale dTEC x-axis to match SBD range
                dtec_x_rescaled.resize(dtec_x.size());
                if(dtec_x.size() > 1)
                {
                    double dtec_x_range = dtec_x.back() - dtec_x.front();
                    double target_range = x_max - x_min;

                    for(size_t i = 0; i < dtec_x.size(); ++i)
                    {
                        // Normalize to 0-1, then scale to target range
                        double normalized = (dtec_x[i] - dtec_x.front()) / dtec_x_range;
                        dtec_x_rescaled[i] = x_min + normalized * target_range;
                    }
                }
                else
                {
                    // Single point case
                    dtec_x_rescaled[0] = (x_min + x_max) / 2.0;
                }
            }
        }
        catch(const std::exception& e)
        {
            msg_warn("plot", "Failed to parse dTEC data: " << e.what() << eom);
        }
    }

    // Use the combined y-limits we calculated earlier (before any auto-expansion)
    std::array< double, 2 > proper_y_limits = {y_min, y_max};

    // Plot SBD data (green) - use original x-axis
    auto sbd_line = matplot::plot(sbd_x, sbd_amp, "g-");
    sbd_line->line_width(0.5f);
    sbd_line->color({34/255.0, 139/255.0, 34/255.0}); //forest green

    msg_debug("plot", "Using combined SBD+dTEC Y limits: " << proper_y_limits[0] << " to " << proper_y_limits[1] << eom);

    // Plot dTEC data (red) on the same y-axis scale
    if(!dtec_y.empty() && !dtec_x_rescaled.empty())
    {
        auto dtec_line = matplot::plot(dtec_x_rescaled, dtec_y, "r-");
        dtec_line->line_width(0.5f);

        // Force y-axis to use original data limits (no auto-expansion)
        if(proper_y_limits.size() >= 2)
        {
            try
            {
                auto current_ax = matplot::gca();
                if(current_ax)
                {
                    // Set y-axis to original data limits
                    matplot::ylim({proper_y_limits[0], proper_y_limits[1]});
                    current_ax->y_axis().limits_mode_auto(false);

                    msg_debug("plot", "Set SBD/dTEC Y axis to combined data limits: " << proper_y_limits[0] << " to "
                                                                                      << proper_y_limits[1] << eom);
                }
            }
            catch(const std::exception& e)
            {
                msg_warn("plot", "Failed to fix SBD y-axis: " << e.what() << eom);
            }
        }

        // Add twin x-axis for dTEC scale
        try
        {
            // Get plot area bounds after fixing limits
            auto y_lim = matplot::ylim();
            auto x_lim = matplot::xlim();

            if(y_lim.size() >= 2 && x_lim.size() >= 2)
            {
                double y_top = y_lim[1];
                double y_bottom = y_lim[0];
                double y_range = y_top - y_bottom;
                double x_left = x_lim[0];
                double x_right = x_lim[1];
                double x_range = x_right - x_left;

                // Position elements at the top edge
                double tick_mark_y = y_top - 0.03 * y_range;  // Tick marks extend 3% down from top edge
                double tick_label_y = y_top - 0.05 * y_range; // Labels 5% down from top edge
                double axis_label_y = y_top - 0.08 * y_range; // Axis label 8% down from top edge
    
                std::vector< double > tick_x_positions;
                std::vector< double > tick_values;
                std::vector<double> tspace = {0.1, 0.5, 1, 5, 10, 25, 50, 100, 200, 500}; //target spacing
                double dtec_min = dtec_x.front();
                double dtec_max = dtec_x.back();
                double tick_spacing = determine_desired_tick_spacing(5, dtec_min, dtec_max, tspace);
                double orig_dtec_range = dtec_max - dtec_min;

                // Find the first tick mark (round down to nearest spacing interval)
                double first_tick = std::floor(dtec_min / tick_spacing) * tick_spacing;

                // Generate tick marks at regular intervals
                for(double tick_val = first_tick; tick_val <= dtec_max + tick_spacing * 0.001; tick_val += tick_spacing)
                {
                    // Only include ticks within the data range
                    if(tick_val >= dtec_min && tick_val <= dtec_max)
                    {
                        // Calculate position on rescaled axis
                        double t = (tick_val - dtec_min) / orig_dtec_range;

                        // Map to rescaled x-coordinate with small inset for visibility
                        double dtec_range = dtec_x_rescaled.back() - dtec_x_rescaled.front();
                        double inset = dtec_range * 0.01; // 1% inset from edges
                        double dtec_left = dtec_x_rescaled.front() + inset;
                        double dtec_right = dtec_x_rescaled.back() - inset;
                        double dtec_span = dtec_right - dtec_left;

                        double tick_pos = dtec_left + t * dtec_span;
                        tick_x_positions.push_back(tick_pos);
                        tick_values.push_back(tick_val);
                    }
                }

                // Draw tick marks and labels
                for(size_t i = 0; i < tick_x_positions.size() && i < tick_values.size(); ++i)
                {
                    double tick_x = tick_x_positions[i];
                    double tick_val = tick_values[i];

                    // Draw tick mark (vertical line)
                    std::vector< double > tick_line_x = {tick_x, tick_x};
                    std::vector< double > tick_line_y = {y_top, tick_mark_y};
                    auto tick_mark = matplot::plot(tick_line_x, tick_line_y, "-");
                    tick_mark->color("red"); // Red to match dTEC line
                    tick_mark->line_width(1.0f);

                    // Add tick label centered on tick mark
                    std::ostringstream tick_stream;
                    tick_stream << tick_val;
                    auto tick_text = fLastAxis->text(tick_x, tick_label_y, tick_stream.str());
                    tick_text->color("red");
                    tick_text->font_size(7);
                    tick_text->alignment(matplot::labels::alignment::center);
                }

                // Skip axis label inside plot - will add it in separate text area
                msg_debug("plot", "Added twin x-axis for dTEC range: " << dtec_x.front() << " to " << dtec_x.back() << eom);
            }
        }
        catch(const std::exception& e)
        {
            msg_warn("plot", "Failed to add dTEC twin x-axis: " << e.what() << eom);
            // Fall back to simple label
            fLastAxis->text(0.5, 1.1, "ion. TEC");
        }
    }

    ConstructYTitle(fSubplotConfig["sbd_amp_ytitle"], "amplitude", "#228B22", 8);

    // Configure axis properties  
    auto ax_handle = fLastAxis;//matplot::gca();
    ax_handle->y2_axis().visible(false); // Hide the right-side y-axis (y2) if it exists
    ax_handle->minor_grid(true); // Enable minor grid lines as substitute for minor ticks

    // Add twin x-axis label in separate text area above the plot (safe approach)
    if(!dtec_x.empty())
    {
        ConstructXTitle(fSubplotConfig["ion_tec_title"], "ion. TEC", "red", 8, 0.5, 0.5);
    }

    ConstructXTitle(fSubplotConfig["sbd_title"], "singleband delay ({/Symbol m})", "#228B22", 9, 0.5, 0.0);
}

void MHO_BasicPlotVisitor::make_xpower_plot(const mho_json& plot_dict)
{
    msg_debug("plot", "creating cross-power spectrum plot" << eom);

    // Extract cross-power spectrum data
    auto xpspec_abs = MHO_PlotDataExtractor::extract_vector(plot_dict, "XPSPEC-ABS");
    auto xpspec_arg = MHO_PlotDataExtractor::extract_vector(plot_dict, "XPSPEC-ARG");
    auto xpow_x = MHO_PlotDataExtractor::extract_vector(plot_dict, "XPSPEC_XAXIS");

    if(xpow_x.empty() && !xpspec_abs.empty())
    {
        xpow_x = MHO_PlotDataExtractor::create_index_vector(xpspec_abs.size() / 2);
    }

    if(xpspec_abs.empty() || xpow_x.empty())
    {
        msg_warn("plot", "No cross-power spectrum data available" << eom);
        return;
    }

    // Use subplot2grid for XPower plot
    auto ax = subplot2grid_wrapper(fSubplotConfig["xpower_plot"]);

    // Truncate data to match x-axis length
    size_t data_len = std::min(xpspec_abs.size(), xpow_x.size());
    std::vector< double > xpspec_abs_trunc(xpspec_abs.begin(), xpspec_abs.begin() + data_len);
    std::vector< double > xpspec_arg_trunc(xpspec_arg.begin(), xpspec_arg.begin() + data_len);

    // Plot amplitude (blue circles)
    auto amp_line = matplot::plot(xpow_x, xpspec_abs_trunc, "co-");
    amp_line->marker_size(2.0f);
    amp_line->line_width(0.5f);
    amp_line->marker_color("blue");

    matplot::xlim({xpow_x.front(), xpow_x.back()});
    matplot::ylabel("amplitude");
    
    //change the label font sizes
    ax->font_size(8);
    ax->y_axis().label_font_size(8);
    ax->x_axis().label_font_size(8);

    // Configure axis properties - enable minor grid and rotate y-tick labels
    auto ax_handle = matplot::gca();
    ax_handle->minor_grid(true); // Enable minor grid lines as substitute for minor ticks

    // Plot phase (red circles) on secondary y-axis
    if(!xpspec_arg_trunc.empty())
    {
        fLastAxis->hold(matplot::on);
        auto phase_line = matplot::plot(xpow_x, xpspec_arg_trunc, "ro");
        phase_line->marker_size(2.0f);
        phase_line->line_width(0.5f);
        phase_line->use_y2(true);

        // Set phase axis limits (-180 to 180 degrees)
        auto ax_handle = matplot::gca();
        ax_handle->font_size(8);
        ax_handle->y2_axis().limits({-180, 180});
        ax_handle->y2_axis().label_font_size(8);
        ax_handle->y2_axis().label_color("red");
        ax_handle->y2_axis().tick_values({-180, -90, 0, 90, 180});
    }

    ConstructXTitle(fSubplotConfig["xpower_xtitle"], "Avgd XPow Spectrum (MHz)", "black", 9, 0.5, 0.0);
    ConstructYTitle(fSubplotConfig["xpower_phase_ytitle"], "phase [deg]", "red", 8, true);
}

void MHO_BasicPlotVisitor::make_channel_segment_plots(const mho_json& plot_dict)
{
    msg_debug("plot", "creating channel segment plots" << eom);

    int n_seg = MHO_PlotDataExtractor::extract_int(plot_dict, "NSeg", 1);
    int n_plots = MHO_PlotDataExtractor::extract_int(plot_dict, "NPlots", 1);

    // Extract segment amplitude and phase data
    auto seg_amp = MHO_PlotDataExtractor::extract_vector(plot_dict, "SEG_AMP");
    auto seg_phs = MHO_PlotDataExtractor::extract_vector(plot_dict, "SEG_PHS");

    if(seg_amp.empty() || seg_phs.empty())
    {
        msg_warn("plot", "no segment data available" << eom);
        return;
    }

    // Convert phase from radians to degrees
    std::vector< double > seg_phs_deg(seg_phs.size());
    std::transform(seg_phs.begin(), seg_phs.end(), seg_phs_deg.begin(), [](double rad) { return rad * 180.0 / M_PI; });

    // Get amplitude scaling factor
    double amp_scale = 3.0; // Default scaling factor
    if(plot_dict.contains("Amp"))
    {
        try
        {
            double amp_val = MHO_PlotDataExtractor::extract_double(plot_dict, "Amp", 1.0);
            amp_scale = amp_val * 3.0;
        }
        catch(const std::exception& e)
        {
            msg_warn("plot", "failed to parse Amp value: " << e.what() << eom);
        }
    }

    // Create channel labels
    std::vector< std::string > channel_labels;
    if(plot_dict.contains("ChannelsPlotted"))
    {
        try
        {
            channel_labels = plot_dict["ChannelsPlotted"].get< std::vector< std::string > >();
        }
        catch(const std::exception& e)
        {
            msg_warn("plot", "Failed to parse channel labels: " << e.what() << eom);
        }
    }

    // Generate default labels if not available
    if(channel_labels.empty())
    {
        std::string default_labels = "abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ0123456789$%";
        for(int i = 0; i < n_plots && i < static_cast< int >(default_labels.length()); ++i)
        {
            channel_labels.push_back(std::string(1, default_labels[i]));
        }
    }

    // Skip "All" channel for individual channel plots if we only have 1 channel 
    int n_channel_plots = n_plots;
    if(n_plots == 2){n_channel_plots = n_plots - 1;}

    if(static_cast< int >(seg_amp.size()) >= n_seg * n_plots && static_cast< int >(seg_phs_deg.size()) >= n_seg * n_plots &&
       n_channel_plots > 0)
    {
        for(int ch = 0; ch < n_channel_plots; ++ch)
        {

            auto ch_ax = subplot2grid({35, n_channel_plots}, {16, ch} , 4, 1, fLeftMargin, fRightMargin);
            ch_ax->font_size(8);

            // Extract data for this channel
            std::vector< double > ch_amp(n_seg);
            std::vector< double > ch_phs(n_seg);
            std::vector< double > seg_indices = MHO_PlotDataExtractor::create_index_vector(n_seg);

            for(int seg = 0; seg < n_seg; ++seg)
            {
                int idx = seg * n_plots + ch;
                if(idx < static_cast< int >(seg_amp.size()))
                {
                    ch_amp[seg] = seg_amp[idx];
                }
                if(idx < static_cast< int >(seg_phs_deg.size()))
                {
                    ch_phs[seg] = seg_phs_deg[idx];
                }
            }

            // Plot amplitude (blue circles with lines)
            auto amp_line = matplot::plot(seg_indices, ch_amp, "co-");
            amp_line->marker_size(2.0f);
            amp_line->line_width(0.5f);
            amp_line->marker_color("blue");

            matplot::xlim({0, static_cast< double >(n_seg)});
            matplot::ylim({0, amp_scale});

            // Add phase plot on secondary y-axis
            fLastAxis->hold(matplot::on);
            auto phs_line = matplot::plot(seg_indices, ch_phs, "ro");
            phs_line->marker_size(2.0f);
            phs_line->line_width(0.5f);
            phs_line->use_y2(true);

            // Set phase axis limits
            auto ax_handle = matplot::gca();
            ax_handle->y2_axis().limits({-180, 180});
            ax_handle->font_size(6);

            // Configure axis labels and ticks AFTER all plotting is done
            // Only show y-axis labels on first (leftmost) channel
            bool plot_y1ax = false;
            bool plot_y2ax = false;
            if(ch == 0){ plot_y1ax = true; }
            if(ch == n_channel_plots - 1){plot_y2ax = true;}

            if(plot_y1ax)
            {
                //ax_handle->y_axis().label("amplitude");
                ax_handle->y_axis().label_font_size(6);
                ax_handle->y_axis().label_color("blue");
            }
            else 
            {
                ax_handle->y_axis().visible(false);
                ax_handle->y_axis().ticklabels({});
            }
            
            if(plot_y2ax)
            {
                // Only show phase ylabel on last (rightmost) subplot
                ax_handle->y2_axis().label_font_size(6);
                ax_handle->y2_axis().label_color("red");
                ax_handle->y2_axis().tick_values({-180, -90, 0, 90, 180});
            }
            else 
            {
                ax_handle->y2_axis().visible(false);
                ax_handle->y2_axis().ticklabels({});
            }

            // Hide x-axis labels on all channel plots (they're not meaningful)
            matplot::xticklabels({});

            try
            {
                //add the channel column labels
                auto text_ax = subplot2grid({72, n_channel_plots}, {31, ch} , 1, 1, fLeftMargin, fRightMargin);

                // Turn off axis display for text subplot
                text_ax->x_axis().visible(false);
                text_ax->y_axis().visible(false);
                text_ax->box(false);
                // Set up coordinate system for text placement (0-1 range)
                matplot::xlim({0, 1});
                matplot::ylim({0, 1});

                // Add centered axis label
                auto chan_label = fLastAxis->text(0.5, 0.0, channel_labels[ch]);
                chan_label->font_size(9);
                chan_label->alignment(matplot::labels::alignment::center);
                msg_debug("plot", "Added twin x-axis label above plot" << eom);
            }
            catch(const std::exception& e)
            {
                msg_warn("plot", "Failed to add twin x-axis label above plot: " << e.what() << eom);
            }


            ConstructYTitle(fSubplotConfig["channel_phase_ytitle"], "phase [deg]", "red", 8, true);
            ConstructYTitle(fSubplotConfig["channel_amp_ytitle"], "amplitude", "blue", 8);
        }
    }
}

void MHO_BasicPlotVisitor::make_channel_segment_validity_plots(const mho_json& plot_dict)
{
    msg_debug("plot", "Creating channel segment validity plots" << eom);
    
    int n_seg = MHO_PlotDataExtractor::extract_int(plot_dict, "NSeg", 1);
    int n_plots = MHO_PlotDataExtractor::extract_int(plot_dict, "NPlots", 1);

    // Extract segment amplitude and phase data
    auto usb_frac = MHO_PlotDataExtractor::extract_vector(plot_dict, "SEG_FRAC_USB");
    auto lsb_frac = MHO_PlotDataExtractor::extract_vector(plot_dict, "SEG_FRAC_LSB");

    // Calculate the number of channel plots (exclude last channel as per Python implementation)
    int n_channel_plots = n_plots - 1; //remove the 'All' channel
    int total_channel_slots = n_plots;
    if(n_channel_plots == 1){total_channel_slots = 1;}
    
    if(n_channel_plots <= 0) {
        msg_debug("plot", "No validity plots needed (n_channel_plots <= 0)" << eom);
        return;
    }
    
    // Create separate validity plot box for each channel, aligned with channel plots above
    // Position them at row 20 with height 1, between channel segments (row 16-19) and pcal plots (row 21+)
    for(int ch = 0; ch < n_channel_plots; ++ch)
    {
        try 
        {
            // Create subplot for this channel's validity plot (aligned with channel plots)
            auto validity_ax = subplot2grid({35, total_channel_slots}, {20, ch} , 1, 1, fLeftMargin, fRightMargin);

            // Set up plotting area and hold for multiple plots
            fLastAxis->hold(matplot::on);

            // Extract data for this channel
            std::vector< double > usb_validity(n_seg);
            std::vector< double > lsb_validity(n_seg);
            std::vector< double > seg_indices = MHO_PlotDataExtractor::create_index_vector(n_seg);

            for(int seg = 0; seg < n_seg; ++seg)
            {
                int idx = seg * n_channel_plots + ch;
                
                if(idx < static_cast< int >(usb_frac.size()))
                {
                    usb_validity[seg] = usb_frac[idx];
                }
                if(idx < static_cast< int >(lsb_frac.size()))
                {
                    lsb_validity[seg] = lsb_frac[idx];
                }
            }

            // Draw vertical lines for each time segment
            for(int seg = 0; seg < n_seg; ++seg) 
            {
                double x = static_cast<double>(seg) + 0.5; // Center the line in the segment
                
                //threshold is hard-coded to 0.95 (see generate_graphs.c)
                if (usb_validity[seg] >= 0.95) 
                {
                    auto usb_line = matplot::plot({x, x}, {0.5, 1.0}, "g-"); // Green for valid
                    usb_line->line_width(1.0f);
                    usb_line->color({34/255.0, 139/255.0, 34/255.0});
                } 
                else if (usb_validity[seg] < 0.95 && usb_validity[seg] > 0. ) 
                {
                    auto usb_line = matplot::plot({x, x}, {0.5, 1.0}, "r-"); // Red for invalid
                    usb_line->line_width(1.0f);
                }
                
                // LSB validity line (lower half: y from 0.0 to 0.5) - should be GREEN
                if (lsb_validity[seg] >= 0.95) 
                {
                    auto lsb_line = matplot::plot({x, x}, {0.0, 0.5}, "g-"); // Green for valid
                    lsb_line->line_width(1.0f);
                    lsb_line->color({34/255.0, 139/255.0, 34/255.0});
                } 
                else if (lsb_validity[seg] < 0.95 && lsb_validity[seg] > 0. ) 
                {
                    auto lsb_line = matplot::plot({x, x}, {0.0, 0.5}, "r-"); // Red for invalid
                    lsb_line->line_width(1.0f);
                }
            }
            
            // Draw horizontal divider line at y=0.5 to separate USB/LSB
            auto divider = matplot::plot({0.0, static_cast<double>(n_seg)}, {0.5, 0.5}, "k-");
            divider->line_width(1.0f);
            
            // Set axis properties
            matplot::xlim({0, static_cast<double>(n_seg)});
            matplot::ylim({0, 1});
            
            // Hide tick labels and ticks as per Python implementation
            auto ax_handle = matplot::gca();
            ax_handle->x_axis().tick_values({});
            ax_handle->y_axis().tick_values({});
            
            // Turn off hold for next plot
            matplot::hold(matplot::off);


            // USB/LSB indicators
            try
            {
                // Create a small text-only subplot above the main plot for the axis label
                auto text_ax = subplot2grid_wrapper(fSubplotConfig["usblsb_frac"]);

                // Turn off axis display for text subplot
                text_ax->x_axis().visible(false);
                text_ax->y_axis().visible(false);
                text_ax->box(false);
                // Set up coordinate system for text placement (0-1 range)
                matplot::xlim({0, 1});
                matplot::ylim({0, 1});
                auto usb_txt = fLastAxis->text(0.5, 0.9, "U");
                usb_txt->alignment(matplot::labels::alignment::center);
                usb_txt->font_size(7);
                auto lsb_txt = fLastAxis->text(0.5, 0.1, "L");
                lsb_txt->alignment(matplot::labels::alignment::center);
                lsb_txt->font_size(7);

            }
            catch(const std::exception& e)
            {
                msg_warn("plot", "Failed to add twin x-axis label above plot: " << e.what() << eom);
            }



        }
        catch(const std::exception& e) 
        {
            msg_warn("plot", "failed to create validity plot for channel " << ch << ": " << e.what() << eom);
        }
    }
}

void MHO_BasicPlotVisitor::make_pcal_plots(const mho_json& plot_dict)
{
    msg_debug("plot", "Creating phase calibration plots" << eom);

    int n_seg = MHO_PlotDataExtractor::extract_int(plot_dict, "NSeg", 1);
    int n_plots = MHO_PlotDataExtractor::extract_int(plot_dict, "NPlots", 1);

    // Check for plot info section
    if(!plot_dict.contains("PLOT_INFO"))
    {
        msg_warn("plot", "No PLOT_INFO section available for pcal plots" << eom);
        return;
    }

    auto plot_info = plot_dict["PLOT_INFO"];

    // Extract manual pcal offsets
    std::vector< double > ref_pcal_off, rem_pcal_off;
    if(plot_info.contains("PCOffRf"))
    {
        ref_pcal_off = MHO_PlotDataExtractor::extract_vector(plot_info, "PCOffRf");
    }
    if(plot_info.contains("PCOffRm"))
    {
        rem_pcal_off = MHO_PlotDataExtractor::extract_vector(plot_info, "PCOffRm");
    }

    if(ref_pcal_off.empty() && rem_pcal_off.empty())
    {
        msg_warn("plot", "No pcal offset data available" << eom);
        return;
    }

    // Skip "All" channel for individual channel plots
    int n_channel_plots = n_plots - 1;
    int total_channel_slots = n_plots;
    if(n_channel_plots == 1){total_channel_slots = 1;}
    
    if(!ref_pcal_off.empty() || !rem_pcal_off.empty())
    {
        for(int ch = 0; ch < n_channel_plots; ++ch)
        {
            auto pcal_ax = subplot2grid({35, total_channel_slots}, {21, ch} , 2, 1, fLeftMargin, fRightMargin);
            pcal_ax->font_size(8);

            std::vector< double > seg_indices = MHO_PlotDataExtractor::create_index_vector(n_seg);

            // Get pcal data for this channel
            double ref_val = (ch < static_cast< int >(ref_pcal_off.size())) ? ref_pcal_off[ch] : 0.0;
            double rem_val = (ch < static_cast< int >(rem_pcal_off.size())) ? rem_pcal_off[ch] : 0.0;

            std::vector< double > ref_phases(n_seg, ref_val);
            std::vector< double > rem_phases(n_seg, rem_val);

            // Check for segment-wise pcal data in "extra" section
            if(plot_dict.contains("extra"))
            {
                try
                {
                    if(plot_dict["extra"].contains("ref_mtpc_phase_segs") &&
                       ch < static_cast< int >(plot_dict["extra"]["ref_mtpc_phase_segs"].size()))
                    {
                        ref_phases = plot_dict["extra"]["ref_mtpc_phase_segs"][ch].get< std::vector< double > >();
                    }
                    if(plot_dict["extra"].contains("rem_mtpc_phase_segs") &&
                       ch < static_cast< int >(plot_dict["extra"]["rem_mtpc_phase_segs"].size()))
                    {
                        rem_phases = plot_dict["extra"]["rem_mtpc_phase_segs"][ch].get< std::vector< double > >();
                    }
                }
                catch(const std::exception& e)
                {
                    msg_warn("plot", "Failed to parse segment pcal data for channel " << ch << ": " << e.what() << eom);
                }
            }

            // Plot reference station pcal (green)
            auto ref_line = matplot::plot(seg_indices, ref_phases, "co");
            ref_line->marker_size(2.0f);
            ref_line->line_width(0.5f);
            ref_line->color({34/255.0, 139/255.0, 34/255.0}); //forest green
            ref_line->marker_color({34/255.0, 139/255.0, 34/255.0});
            fLastAxis->hold(matplot::on);

            // Plot remote station pcal (magenta)
            auto rem_line = matplot::plot(seg_indices, rem_phases, "co");
            rem_line->marker_size(2.0f);
            rem_line->line_width(0.5f);
            rem_line->color("magenta");
            rem_line->marker_color("magenta");

            matplot::xlim({0, static_cast< double >(n_seg)});
            matplot::ylim({-180, 180});

            // Configure axis labels and ticks
            // Only show y-axis labels on first (leftmost) channel
            if(ch == 0)
            {
                matplot::yticklabels({});
                auto ax_handle = matplot::gca();
                ax_handle->y_axis().tick_values({-180, -90, 0, 90, 180});
                ax_handle->font_size(6);
            }
            else if(ch == n_channel_plots-1)
            {
                auto ax_handle = matplot::gca();
                matplot::yticklabels({});
                ax_handle->y2_axis().tick_values({-180, -90, 0, 90, 180});
                ax_handle->font_size(6);
            }
            else
            {
                // Hide y-axis tick labels on middle channels using empty vector
                matplot::yticklabels({});
            }
            //hide all x axis ticks
            matplot::xticklabels({});
        }
    }

    if(total_channel_slots == 1)
    {
        fSubplotConfig["pcal_theta_ytitle"] = subplot_parameters(2*fNRows, 2*fNCols, 40, 125, 8, 1);
        fSubplotConfig["station_codes"] = subplot_parameters(4*fNRows, 2*fNCols, 87, 125, 2, 1);
    }

    ConstructYTitle(fSubplotConfig["pcal_theta_ytitle"], "pcal {/Symbol q}", "black", 8);

    // Add station identifiers if available
    if(plot_dict.contains("extra"))
    {
        //add station indicators
        try
        {
            // Create a small text-only subplot above the main plot for the axis label
            auto text_ax = subplot2grid_wrapper(fSubplotConfig["station_codes"]);

            // Turn off axis display for text subplot
            text_ax->x_axis().visible(false);
            text_ax->y_axis().visible(false);
            text_ax->box(false);
            // Set up coordinate system for text placement (0-1 range)
            matplot::xlim({0, 1});
            matplot::ylim({0, 1});

            std::string ref_id = MHO_PlotDataExtractor::extract_string(plot_dict["extra"], "ref_station_mk4id", "REF");
            std::string rem_id = MHO_PlotDataExtractor::extract_string(plot_dict["extra"], "rem_station_mk4id", "REM");

            auto ref_txt = fLastAxis->text(0.5, 0.0, ref_id);
            ref_txt->color("#228B22"); //forest green
            ref_txt->alignment(matplot::labels::alignment::center);

            auto rem_txt = fLastAxis->text(0.5, 1.0, rem_id);
            rem_txt->alignment(matplot::labels::alignment::center);
            rem_txt->color("magenta");

        }
        catch(const std::exception& e)
        {
            msg_warn("plot", "Failed to add twin x-axis label above plot: " << e.what() << eom);
        }

    }
}

// ============================================================================
// Basic Text Annotations
// ============================================================================

void MHO_BasicPlotVisitor::make_top_info_text(const mho_json& plot_dict)
{
    msg_debug("plot", "Adding top info text" << eom);

    // Extract key information for top text
    std::string baseline = MHO_PlotDataExtractor::extract_string(plot_dict, "RootScanBaseline", "Unknown");
    std::string corr_vers = MHO_PlotDataExtractor::extract_string(plot_dict, "CorrVers", "Unknown");
    std::string pol_str = MHO_PlotDataExtractor::extract_string(plot_dict, "PolStr", "Unknown");

    // Remove quotes if present
    baseline.erase(std::remove(baseline.begin(), baseline.end(), '\''), baseline.end());
    corr_vers.erase(std::remove(corr_vers.begin(), corr_vers.end(), '\''), corr_vers.end());
    pol_str.erase(std::remove(pol_str.begin(), pol_str.end(), '\''), pol_str.end());

    // Create a figure-level text area for positioning (similar to Python's figure coordinates)
    // Use rows 0-1 at the top for text positioning with reduced margin (~5-6mm at 850px height)
    auto text_ax = subplot2grid_wrapper(fSubplotConfig["top_info_textbox"]);

    // Turn off axis display for text subplot
    text_ax->x_axis().visible(false);
    text_ax->y_axis().visible(false);
    text_ax->box(false);

    // Set up coordinate system for text placement (0-1 range)
    matplot::xlim({0, 1});
    matplot::ylim({0, 1});

    // Helper lambda for right-justified text
    auto right_justify_text = [](matplot::axes_handle ax, double right_x, double y, const std::string& text) {
        double char_width = 0.0095; // Smaller character width for top text
        double text_width = text.length() * char_width;
        double left_x = right_x - text_width;
        auto txt = ax->text(left_x, y, text);
        return txt;
    };

    auto corr_text = fLastAxis->text(0.025, 0.85, corr_vers);
    corr_text->font("Arial Black,11");

    // Top-right, upper (0.965, 0.98): RootScanBaseline
    auto baseline_text = right_justify_text(fLastAxis, 0.95, 0.85, baseline);
    baseline_text->font("Arial Black,12");

    // Top-left, lower (0.05, 0.96): NOT FOR PRODUCTION warning
    auto warning_text = fLastAxis->text(0.025, 0.5, "NOT FOR PRODUCTION");
    warning_text->font("Arial Black,11");
    warning_text->color("red");

    // Top-right, lower (0.965, 0.96): PolStr
    auto pol_text = right_justify_text(fLastAxis, 0.965, 0.5, pol_str);
    pol_text->font("Arial Black,11");
}

void MHO_BasicPlotVisitor::make_basic_info_text(const mho_json& plot_dict)
{
    msg_debug("plot", "Adding basic info text" << eom);

    // Create text area for info box (positioned like Python at right side)
    // Info text box with borders - right side with border (reduced height to avoid collision)
    auto text_ax = subplot2grid_wrapper(fSubplotConfig["basic_info_textbox"]);

    // Turn off axis display for text subplot
    text_ax->x_axis().visible(false);
    text_ax->y_axis().visible(false);
    text_ax->box(false);

    // Set up coordinate system for text placement (0-1 range)
    matplot::xlim({0, 1});
    matplot::ylim({0, 1});

    // Extract all data fields following the Python implementation
    std::string quality = MHO_PlotDataExtractor::extract_string(plot_dict, "Quality", "?");
    // Remove quotes if present
    quality.erase(std::remove(quality.begin(), quality.end(), '\''), quality.end());

    std::string snr = MHO_PlotDataExtractor::format_standard(MHO_PlotDataExtractor::extract_double(plot_dict, "SNR", 0.0), 1, 1);
    std::string intg_time =
        MHO_PlotDataExtractor::format_standard(MHO_PlotDataExtractor::extract_double(plot_dict, "IntgTime", 0.0), 3, 3);
    std::string amp = MHO_PlotDataExtractor::format_standard(MHO_PlotDataExtractor::extract_double(plot_dict, "Amp", 0.0), 3, 3);
    std::string phase =
        MHO_PlotDataExtractor::format_standard(MHO_PlotDataExtractor::extract_double(plot_dict, "ResPhase", 0.0), 1, 1);
    std::string pfd = MHO_PlotDataExtractor::format_standard(MHO_PlotDataExtractor::extract_double(plot_dict, "PFD", 0.0), 1, 1);

    std::string sbd =
        MHO_PlotDataExtractor::format_standard(MHO_PlotDataExtractor::extract_double(plot_dict, "ResidSbd(us)", 0.0), 6, 6);
    std::string mbd =
        MHO_PlotDataExtractor::format_standard(MHO_PlotDataExtractor::extract_double(plot_dict, "ResidMbd(us)", 0.0), 6, 6);
    std::string fringe_rate =
        MHO_PlotDataExtractor::format_standard(MHO_PlotDataExtractor::extract_double(plot_dict, "FringeRate(Hz)", 0.0), 6, 6);

    std::string ion_tec =
        MHO_PlotDataExtractor::format_standard(MHO_PlotDataExtractor::extract_double(plot_dict, "IonTEC(TEC)", 0.0), 3, 3);
    std::string ref_freq =
        MHO_PlotDataExtractor::format_standard(MHO_PlotDataExtractor::extract_double(plot_dict, "RefFreq(MHz)", 0.0), 4, 4);
    std::string ap_sec =
        MHO_PlotDataExtractor::format_standard(MHO_PlotDataExtractor::extract_double(plot_dict, "AP(sec)", 0.0), 3, 3);

    std::string exp_name = MHO_PlotDataExtractor::extract_string(plot_dict, "ExperName", "");
    std::string exp_num = std::to_string(MHO_PlotDataExtractor::extract_int(plot_dict, "ExperNum", 0));
    std::string year_doy = MHO_PlotDataExtractor::extract_string(plot_dict, "YearDOY", "");
    std::string start_time = MHO_PlotDataExtractor::extract_string(plot_dict, "Start", "");
    std::string stop_time = MHO_PlotDataExtractor::extract_string(plot_dict, "Stop", "");
    std::string frt = MHO_PlotDataExtractor::extract_string(plot_dict, "FRT", "");

    std::string corr_time = MHO_PlotDataExtractor::extract_string(plot_dict, "CorrTime", "");
    std::string ff_time = MHO_PlotDataExtractor::extract_string(plot_dict, "FFTime", "");
    std::string build_time = MHO_PlotDataExtractor::extract_string(plot_dict, "BuildTime", "");

    std::string ra = MHO_PlotDataExtractor::extract_string(plot_dict, "RA", "");
    std::string dec = MHO_PlotDataExtractor::extract_string(plot_dict, "Dec", "");
    // Replace 'd' with degree symbol (simplified as 'd' for now)
    // if(dec.find("d") != std::string::npos){ dec.replace(dec.find("d"), 1, "°"); }
    //we use a superscript 'o' as a stand-in for the degree symbol, because not all
    //of the gnuplot backends support UTF-8 code points, and we don't know what backend is being used here
    if(dec.find("d") != std::string::npos){ dec.replace(dec.find("d"), 1, "{^o}"); }

    // Check for error code
    std::string error_code = "";
    if(plot_dict.contains("extra") && plot_dict["extra"].contains("error_code"))
    {
        error_code = MHO_PlotDataExtractor::extract_string(plot_dict["extra"], "error_code", "");
    }

    // Create labels column (left-aligned)
    double y_start = 0.999;
    double y_step = 0.025; // Smaller step for more compact layout
    double y_pos = y_start;
    double label_x = 0.1;
    double value_x = 0.85; // Right boundary for right-justified text (moved left from edge)

    // Helper lambda to create green-colored label text
    auto green_label = [label_x](matplot::axes_handle ax, double y, const std::string& text) {
        auto label = ax->text(label_x, y, text);
        label->font_size(9);
        label->font("monospace");
        label->color("#228B22");
        return label;
    };

    // Basic quality section
    green_label(fLastAxis, y_pos, "Fringe quality");
    y_pos -= y_step * 2;
    green_label(fLastAxis, y_pos, "SNR");
    y_pos -= y_step;
    green_label(fLastAxis, y_pos, "Int time");
    y_pos -= y_step;
    green_label(fLastAxis, y_pos, "Amp");
    y_pos -= y_step;
    green_label(fLastAxis, y_pos, "Phase");
    y_pos -= y_step;
    green_label(fLastAxis, y_pos, "PFD");
    y_pos -= y_step;
    green_label(fLastAxis, y_pos, "Delay (us)");
    y_pos -= y_step;
    green_label(fLastAxis, y_pos, "SBD");
    y_pos -= y_step;
    green_label(fLastAxis, y_pos, "MBD");
    y_pos -= y_step;
    green_label(fLastAxis, y_pos, "Fringe rate (Hz)");
    y_pos -= y_step * 2;

    // Technical parameters section
    green_label(fLastAxis, y_pos, "Ion TEC");
    y_pos -= y_step;
    green_label(fLastAxis, y_pos, "Ref freq (MHz)");
    y_pos -= y_step * 2;

    // Experiment/observation section
    green_label(fLastAxis, y_pos, "AP (sec)");
    y_pos -= y_step;
    green_label(fLastAxis, y_pos, "Exp.");
    y_pos -= y_step;
    green_label(fLastAxis, y_pos, "Exper #");
    y_pos -= y_step;
    green_label(fLastAxis, y_pos, "Yr:day");
    y_pos -= y_step;
    green_label(fLastAxis, y_pos, "Start");
    y_pos -= y_step;
    green_label(fLastAxis, y_pos, "Stop");
    y_pos -= y_step;
    green_label(fLastAxis, y_pos, "FRT");
    y_pos -= y_step;
    green_label(fLastAxis, y_pos, "Corr/FF/build");
    y_pos -= y_step * 4;

    // Coordinates section
    green_label(fLastAxis, y_pos, "RA, Dec (J2000)");

    // Lambda function to create right-justified text
    auto right_justify_text = [](matplot::axes_handle ax, double right_x, double y, const std::string& text) 
    {
        auto txt = ax->text(right_x, y, text);
        txt->font_size(9); // Smaller font size
        txt->font("monospace");
        txt->alignment(matplot::labels::alignment::right);
    };

    // Create values column (right-aligned)
    y_pos = y_start;

    right_justify_text(fLastAxis, value_x, y_pos, quality);
    y_pos -= y_step;
    // Add error code if present (red text, right-justified)
    if(!error_code.empty() && error_code != " ")
    {
        right_justify_text(fLastAxis, value_x, y_pos, "Error code " + error_code);
    }
    y_pos -= y_step;
    right_justify_text(fLastAxis, value_x, y_pos, snr);
    y_pos -= y_step;
    right_justify_text(fLastAxis, value_x, y_pos, intg_time);
    y_pos -= y_step;
    right_justify_text(fLastAxis, value_x, y_pos, amp);
    y_pos -= y_step;
    right_justify_text(fLastAxis, value_x, y_pos, phase);
    y_pos -= y_step;
    right_justify_text(fLastAxis, value_x, y_pos, pfd);
    y_pos -= y_step;
    y_pos -= y_step; // Skip "Delay (us)" header
    right_justify_text(fLastAxis, value_x, y_pos, sbd);
    y_pos -= y_step;
    right_justify_text(fLastAxis, value_x, y_pos, mbd);
    y_pos -= y_step;
    y_pos -= y_step; // Skip empty line
    right_justify_text(fLastAxis, value_x, y_pos, fringe_rate);
    y_pos -= y_step;
    right_justify_text(fLastAxis, value_x, y_pos, ion_tec);
    y_pos -= y_step * 2;
    right_justify_text(fLastAxis, value_x, y_pos, ref_freq);
    y_pos -= y_step;
    right_justify_text(fLastAxis, value_x, y_pos, ap_sec);
    y_pos -= y_step;
    right_justify_text(fLastAxis, value_x, y_pos, exp_name);
    y_pos -= y_step;
    right_justify_text(fLastAxis, value_x, y_pos, exp_num);
    y_pos -= y_step;
    right_justify_text(fLastAxis, value_x, y_pos, year_doy);
    y_pos -= y_step;
    right_justify_text(fLastAxis, value_x, y_pos, start_time);
    y_pos -= y_step;
    right_justify_text(fLastAxis, value_x, y_pos, stop_time);
    y_pos -= y_step;
    right_justify_text(fLastAxis, value_x, y_pos, frt);
    y_pos -= y_step * 2;
    right_justify_text(fLastAxis, value_x, y_pos, corr_time);
    y_pos -= y_step;
    right_justify_text(fLastAxis, value_x, y_pos, ff_time);
    y_pos -= y_step;
    right_justify_text(fLastAxis, value_x, y_pos, build_time);
    y_pos -= y_step * 2;
    right_justify_text(fLastAxis, value_x, y_pos, ra);
    y_pos -= y_step;
    right_justify_text(fLastAxis, value_x, y_pos, dec);


}

// ============================================================================
// Additional Text Functions
// ============================================================================

void MHO_BasicPlotVisitor::make_model_resid_info_text(const mho_json& plot_dict)
{
    msg_debug("plot", "Adding model residual info text" << eom);

    // Extract model values
    std::string group_delay = MHO_PlotDataExtractor::format_scientific(
        MHO_PlotDataExtractor::extract_double(plot_dict, "GroupDelayModel(usec)",
                                              MHO_PlotDataExtractor::extract_double(plot_dict, "GroupDelaySBD(usec)", 0.0)),
        11);
    std::string sband_delay =
        MHO_PlotDataExtractor::format_scientific(MHO_PlotDataExtractor::extract_double(plot_dict, "SbandDelay(usec)", 0.0), 11);
    std::string phase_delay =
        MHO_PlotDataExtractor::format_scientific(MHO_PlotDataExtractor::extract_double(plot_dict, "PhaseDelay(usec)", 0.0), 11);
    std::string delay_rate =
        MHO_PlotDataExtractor::format_scientific(MHO_PlotDataExtractor::extract_double(plot_dict, "DelayRate(ps/s)", 0.0), 11);
    std::string total_phase =
        MHO_PlotDataExtractor::format_standard(MHO_PlotDataExtractor::extract_double(plot_dict, "TotalPhase(deg)", 0.0), 1, 1);

    // Extract apriori values
    std::string apr_delay = MHO_PlotDataExtractor::format_scientific(
        MHO_PlotDataExtractor::extract_double(plot_dict, "AprioriDelay(usec)", 0.0), 11);
    std::string apr_clock = MHO_PlotDataExtractor::format_scientific(
        MHO_PlotDataExtractor::extract_double(plot_dict, "AprioriClock(usec)", 0.0), 8);
    std::string apr_clockrate = MHO_PlotDataExtractor::format_scientific(
        MHO_PlotDataExtractor::extract_double(plot_dict, "AprioriClockrate(us/s)", 0.0), 8);
    std::string apr_rate = MHO_PlotDataExtractor::format_scientific(
        MHO_PlotDataExtractor::extract_double(plot_dict, "AprioriRate(us/s)", 0.0), 11);
    std::string apr_accel = MHO_PlotDataExtractor::format_scientific(
        MHO_PlotDataExtractor::extract_double(plot_dict, "AprioriAccel(us/s/s)", 0.0), 11);

    // Extract residual values
    std::string res_mbd =
        MHO_PlotDataExtractor::format_scientific(MHO_PlotDataExtractor::extract_double(plot_dict, "ResidMbd(us)", 0.0), 6);
    std::string res_sbd =
        MHO_PlotDataExtractor::format_scientific(MHO_PlotDataExtractor::extract_double(plot_dict, "ResidSbd(us)", 0.0), 6);
    std::string res_phdelay = MHO_PlotDataExtractor::format_scientific(
        MHO_PlotDataExtractor::extract_double(plot_dict, "ResidPhdelay(usec)", 0.0), 6);
    std::string res_rate =
        MHO_PlotDataExtractor::format_scientific(MHO_PlotDataExtractor::extract_double(plot_dict, "ResidRate(us/s)", 0.0), 6);
    std::string res_phase =
        MHO_PlotDataExtractor::format_standard(MHO_PlotDataExtractor::extract_double(plot_dict, "ResidPhase(deg)", 0.0), 1, 1);

    // Extract error values
    std::string err_mbd = MHO_PlotDataExtractor::format_scientific(
        MHO_PlotDataExtractor::extract_double(plot_dict, "ResidMbdelayError(usec)", 0.0), 1);
    std::string err_sbd = MHO_PlotDataExtractor::format_scientific(
        MHO_PlotDataExtractor::extract_double(plot_dict, "ResidSbdelayError(usec)", 0.0), 1);
    std::string err_phdelay = MHO_PlotDataExtractor::format_scientific(
        MHO_PlotDataExtractor::extract_double(plot_dict, "ResidPhdelayError(usec)", 0.0), 1);
    std::string err_rate = MHO_PlotDataExtractor::format_scientific(
        MHO_PlotDataExtractor::extract_double(plot_dict, "ResidRateError(us/s)", 0.0), 1);
    std::string err_phase = MHO_PlotDataExtractor::format_standard(
        MHO_PlotDataExtractor::extract_double(plot_dict, "ResidPhaseError(deg)", 0.0), 1, 1);

    // Create text area at bottom of plot - use full width without margins
    auto text_ax = subplot2grid_wrapper(fSubplotConfig["model_resid_info_textbox"]);

    // Turn off axis display
    text_ax->x_axis().visible(false);
    text_ax->y_axis().visible(false);
    text_ax->box(false);

    matplot::xlim({0, 1});
    matplot::ylim({0, 1});

    // Following Python implementation: 3-table layout with left-justified labels and right-justified values
    // Create the label strings for each table
    std::vector< std::string > table1_labels = {"Group delay (usec)", "Sband delay (usec)", "Phase delay (usec)",
                                                "Delay rate (us/s)", "Total Phase (deg)"};
    std::vector< std::string > table1_values = {group_delay, sband_delay, phase_delay, delay_rate, total_phase};

    std::vector< std::string > table2_labels = {"Apriori delay (usec)", "Apriori clock (usec)", "Apriori clockrate (us/s)",
                                                "Apriori rate (us/s)", "Apriori accel (us/s/s)"};
    std::vector< std::string > table2_values = {apr_delay, apr_clock, apr_clockrate, apr_rate, apr_accel};

    std::vector< std::string > table3_labels = {"Resid mbdelay (usec)", "Resid sbdelay (usec)", "Resid phdelay (usec)",
                                                "Resid rate (us/s)", "Resid phase (deg)"};
    std::vector< std::string > table3_values = {res_mbd, res_sbd, res_phdelay, res_rate, res_phase};
    std::vector< std::string > plusminus_symbols = {"+/-", "+/-", "+/-", "+/-", "+/-"};
    std::vector< std::string > error_values = {err_mbd, err_sbd, err_phdelay, err_rate, err_phase};

    // Layout parameters - much tighter vertical spacing
    double y_start = 0.9;
    double y_step = 0.08; // Very tight line spacing (was 0.12, now 0.08)

    // Table 1: Model values (left-justified labels, right-justified values)
    double table1_label_x = 0.01; // Python: 0.01
    double table1_value_x = 0.25; // Python: 0.28 (right-justified)

    // Table 2: Apriori values (left-justified labels, right-justified values)
    double table2_label_x = 0.3; // Python: 0.3
    double table2_value_x = 0.57; // Python: 0.6 (right-justified)

    // Table 3: Residual values (left-justified labels, right-justified values)
    double table3_label_x = 0.63; // Python: 0.65
    double table3_value_x = 0.85; // Python: 0.88 (right-justified)
    double plusminus_x = 0.89;    // Python: 0.89
    double error_x = 0.97;        // Python: 0.97 (right-justified)

    // Helper lambda for right-justified text with smaller font size
    auto right_justify_text = [](matplot::axes_handle ax, double right_x, double y, const std::string& text) 
    {
        double char_width = 0.006; // Reduced character width for smaller text
        double text_width = text.length() * char_width;
        double left_x = right_x - text_width;
        auto txt = ax->text(left_x, y, text);
        txt->font_size(8); // Smaller font size
        return txt;
    };

    // Draw all three tables with smaller font size
    for(size_t i = 0; i < table1_labels.size(); ++i)
    {
        double y_pos = y_start - i * y_step;

        // Table 1: Model values
        auto label1 = fLastAxis->text(table1_label_x, y_pos, table1_labels[i]);
        label1->font_size(8); // Smaller font size
        right_justify_text(fLastAxis, table1_value_x, y_pos, table1_values[i]);

        // Table 2: Apriori values
        auto label2 = fLastAxis->text(table2_label_x, y_pos, table2_labels[i]);
        label2->font_size(8); // Smaller font size
        right_justify_text(fLastAxis, table2_value_x, y_pos, table2_values[i]);

        // Table 3: Residual values with errors
        auto label3 = fLastAxis->text(table3_label_x, y_pos, table3_labels[i]);
        label3->font_size(8); // Smaller font size
        right_justify_text(fLastAxis, table3_value_x, y_pos, table3_values[i]);
        auto plus_minus = fLastAxis->text(plusminus_x, y_pos, plusminus_symbols[i]);
        plus_minus->font_size(8); // Smaller font size
        right_justify_text(fLastAxis, error_x, y_pos, error_values[i]);
    }
}

void MHO_BasicPlotVisitor::make_rms_table(const mho_json& plot_dict)
{
    msg_debug("plot", "Adding RMS table" << eom);

    if(!plot_dict.contains("extra"))
    {
        msg_warn("plot", "No extra data available for RMS table" << eom);
        return;
    }

    auto extra = plot_dict["extra"];

    // Extract RMS values
    double theory_timerms_phase = MHO_PlotDataExtractor::extract_double(extra, "theory_timerms_phase", 0.0);
    double theory_timerms_amp = MHO_PlotDataExtractor::extract_double(extra, "theory_timerms_amp", 0.0);
    double theory_freqrms_phase = MHO_PlotDataExtractor::extract_double(extra, "theory_freqrms_phase", 0.0);
    double theory_freqrms_amp = MHO_PlotDataExtractor::extract_double(extra, "theory_freqrms_amp", 0.0);
    double timerms_phase = MHO_PlotDataExtractor::extract_double(extra, "timerms_phase", 0.0);
    double timerms_amp = MHO_PlotDataExtractor::extract_double(extra, "timerms_amp", 0.0);
    double freqrms_phase = MHO_PlotDataExtractor::extract_double(extra, "freqrms_phase", 0.0);
    double freqrms_amp = MHO_PlotDataExtractor::extract_double(extra, "freqrms_amp", 0.0);

    // Create text area for RMS table - positioned at bottom left, increased height
    auto text_ax = subplot2grid_wrapper(fSubplotConfig["rms_textbox"]);

    text_ax->x_axis().visible(false);
    text_ax->y_axis().visible(false);
    text_ax->box(false);

    matplot::xlim({0, 1});
    matplot::ylim({0, 1});

    // Format values to 1 decimal place like Python
    std::ostringstream oss;
    oss << std::fixed << std::setprecision(1);

    // Create table data arrays
    std::vector< std::string > row_labels = {"", "ph/seg (deg)", "amp/seg (%)", "ph/frq (deg)", "amp/frq (%)"};
    std::vector< double > rms_values = {0.0, timerms_phase, timerms_amp, freqrms_phase, freqrms_amp};
    std::vector< double > theory_values = {0.0, theory_timerms_phase, theory_timerms_amp, theory_freqrms_phase,
                                           theory_freqrms_amp};
    std::vector< std::string > col_headers = {"", "RMS", "Theor."};

    // Layout parameters - compact like model_resid_info_text
    double y_start = 0.9;
    double y_step = 0.09; // Tight vertical spacing
    double label_x = 0.02;
    double rms_col_x = 0.4;
    double theory_col_x = 0.6;

    // Helper lambda for right-justified text with small font
    auto right_justify_text = [](matplot::axes_handle ax, double right_x, double y, const std::string& text) {
        double char_width = 0.006;
        double text_width = text.length() * char_width;
        double left_x = right_x - text_width;
        auto txt = ax->text(left_x, y, text);
        txt->font_size(8);
        return txt;
    };

    // Draw table headers
    auto header1 = fLastAxis->text(rms_col_x - 0.02, y_start, "RMS");
    header1->font_size(8);
    auto header2 = fLastAxis->text(theory_col_x - 0.03, y_start, "Theor.");
    header2->font_size(8);

    // Draw table rows
    for(size_t i = 1; i < row_labels.size(); ++i)
    {
        double y_pos = y_start - i * y_step;

        // Row label (left-justified)
        auto label = fLastAxis->text(label_x, y_pos, row_labels[i]);
        label->font_size(8);

        // RMS value (right-justified)
        oss.str("");
        oss.clear();
        oss << rms_values[i];
        right_justify_text(fLastAxis, rms_col_x, y_pos, oss.str());

        // Theory value (right-justified)
        oss.str("");
        oss.clear();
        oss << theory_values[i];
        right_justify_text(fLastAxis, theory_col_x, y_pos, oss.str());
    }
}

void MHO_BasicPlotVisitor::make_coord_text(const mho_json& plot_dict)
{
    msg_debug("plot", "Adding coordinate text" << eom);

    if(!plot_dict.contains("extra"))
    {
        msg_warn("plot", "No extra data available for coordinates" << eom);
        return;
    }

    auto extra = plot_dict["extra"];

    // Extract station information
    std::string ref_station = MHO_PlotDataExtractor::extract_string(extra, "ref_station_mk4id", "REF");
    std::string rem_station = MHO_PlotDataExtractor::extract_string(extra, "rem_station_mk4id", "REM");

    double ref_az = 0.0, ref_el = 0.0, ref_pa = 0.0;
    double rem_az = 0.0, rem_el = 0.0, rem_pa = 0.0;
    double u = 0.0, v = 0.0;

    if(extra.contains("ref_station"))
    {
        ref_az = MHO_PlotDataExtractor::extract_double(extra["ref_station"], "az", 0.0);
        ref_el = MHO_PlotDataExtractor::extract_double(extra["ref_station"], "el", 0.0);
        ref_pa = MHO_PlotDataExtractor::extract_double(extra["ref_station"], "pa", 0.0);
    }

    if(extra.contains("rem_station"))
    {
        rem_az = MHO_PlotDataExtractor::extract_double(extra["rem_station"], "az", 0.0);
        rem_el = MHO_PlotDataExtractor::extract_double(extra["rem_station"], "el", 0.0);
        rem_pa = MHO_PlotDataExtractor::extract_double(extra["rem_station"], "pa", 0.0);
    }

    u = MHO_PlotDataExtractor::extract_double(extra, "u", 0.0);
    v = MHO_PlotDataExtractor::extract_double(extra, "v", 0.0);

    // Control file and input file info
    std::string control_file = MHO_PlotDataExtractor::extract_string(extra, "control_file", "");
    std::string baseline_input = MHO_PlotDataExtractor::extract_string(extra, "baseline_input_file", "");

    //TODO FIXME horrible hack to keep gnuplot from interpreting underscore as subscript indcator
    while(control_file.find("_") != std::string::npos){ control_file.replace(control_file.find("_"), 1, "*"); }
    while(control_file.find("*") != std::string::npos){ control_file.replace(control_file.find("*"), 1, "\\\\_"); }
    while(baseline_input.find("_") != std::string::npos){ baseline_input.replace(baseline_input.find("_"), 1, "*"); }
    while(baseline_input.find("*") != std::string::npos){ baseline_input.replace(baseline_input.find( "*"), 1, "\\\\_"); }

    // Create coordinate text at bottom - give it more height and better positioning
    auto text_ax = subplot2grid_wrapper(fSubplotConfig["coord_textbox"]);

    text_ax->x_axis().visible(false);
    text_ax->y_axis().visible(false);
    text_ax->box(false);

    matplot::xlim({0, 1});
    matplot::ylim({0, 1});

    // Format coordinate text
    std::ostringstream coord_text;
    coord_text << ref_station << ": az " << std::fixed << std::setprecision(1) << ref_az << " el " << ref_el << " pa " << ref_pa
               << "    " << rem_station << ": az " << rem_az << " el " << rem_el << " pa " << rem_pa << "    u,v (fr/asec) "
               << std::setprecision(3) << u << ", " << v;

    auto coord_txt = fLastAxis->text(0.001, 0.8, coord_text.str());
    coord_txt->font_size(8); // Set font size

    // Add file info
    if(!control_file.empty() || !baseline_input.empty())
    {
        auto file_txt = fLastAxis->text(0.001, 0.6, "Control file: " + control_file + "     Input File:" + baseline_input);
        file_txt->font_size(8); // Set font size
    }

    // Add "simultaneous interpolator" text on the right
    auto interp_txt = fLastAxis->text(0.8, 0.8, "simultaneous interpolator");
    interp_txt->font_size(8); // Set font size
}

void MHO_BasicPlotVisitor::make_amplitude_table(const mho_json& plot_dict)
{
    msg_debug("plot", "Adding amplitude table" << eom);

    // Extract amplitude values
    double amp = MHO_PlotDataExtractor::extract_double(plot_dict, "Amp", 0.0);
    double snr = MHO_PlotDataExtractor::extract_double(plot_dict, "SNR", 0.0);

    // Calculate amplitude error (simplified approximation)
    double amp_err = (snr > 0) ? amp / snr : 0.0;

    // Extract additional amplitude data from extra section
    double coarse_search_max = 0.0, inc_avg_amp = 0.0, inc_avg_amp_freq = 0.0;
    int n_drsp_points = 0, grid_pts = 0;

    if(plot_dict.contains("extra"))
    {
        auto extra = plot_dict["extra"];
        n_drsp_points = MHO_PlotDataExtractor::extract_int(extra, "n_drsp_points", 0);
        grid_pts = MHO_PlotDataExtractor::extract_int(extra, "grid_pts", 0);
        coarse_search_max = MHO_PlotDataExtractor::extract_double(extra, "coarse_search_max_amp", 0.0);
        inc_avg_amp = MHO_PlotDataExtractor::extract_double(extra, "inc_avg_amp", 0.0);
        inc_avg_amp_freq = MHO_PlotDataExtractor::extract_double(extra, "inc_avg_amp_freq", 0.0);
    }

    // Create text area for amplitude table - positioned at bottom center, increased height, reduced width
    auto text_ax = subplot2grid_wrapper(fSubplotConfig["amp_table_textbox"]);

    text_ax->x_axis().visible(false);
    text_ax->y_axis().visible(false);
    text_ax->box(false);

    matplot::xlim({0, 1});
    matplot::ylim({0, 1});

    // Prepare data strings
    std::string amp_with_err =
        MHO_PlotDataExtractor::format_standard(amp, 3, 3) + " +/- " + MHO_PlotDataExtractor::format_standard(amp_err, 3, 3);
    std::string search_label = "Search(" + std::to_string(n_drsp_points) + "x" + std::to_string(grid_pts) + ")";
    std::string coarse_max_str = MHO_PlotDataExtractor::format_standard(coarse_search_max, 3, 3);
    std::string inc_seg_str = MHO_PlotDataExtractor::format_standard(inc_avg_amp, 3, 3);
    std::string inc_frq_str = MHO_PlotDataExtractor::format_standard(inc_avg_amp_freq, 3, 3);

    // Table data (single column table like Python)
    std::vector< std::string > labels = {"Amplitude", search_label, "Interp.", "Inc. seg. avg.", "Inc. frq. avg."};
    std::vector< std::string > values = {amp_with_err, coarse_max_str, "0.000", inc_seg_str, inc_frq_str};

    // Layout parameters - compact style
    double y_start = 0.9;
    double y_step = 0.09; // Tight vertical spacing
    double label_x = 0.08;
    double value_x = 0.68; 

    // Draw single-column table
    for(size_t i = 0; i < labels.size(); ++i)
    {
        double y_pos = y_start - i * y_step;

        // Label (left-justified)
        auto label = fLastAxis->text(label_x, y_pos, labels[i]);
        label->font_size(8);
        
        auto txt = fLastAxis->text(value_x, y_pos, values[i]);
        txt->font_size(8);
    }
}

void MHO_BasicPlotVisitor::make_window_table(const mho_json& plot_dict)
{
    msg_debug("plot", "Adding window table" << eom);

    if(!plot_dict.contains("extra"))
    {
        msg_warn("plot", "No extra data available for window table" << eom);
        return;
    }

    auto extra = plot_dict["extra"];

    // Extract window limits
    std::vector< double > sb_win, mb_win, dr_win, ion_win;

    if(extra.contains("sb_win") && extra["sb_win"].is_array() && extra["sb_win"].size() >= 2)
    {
        sb_win = extra["sb_win"].get< std::vector< double > >();
    }
    if(extra.contains("mb_win") && extra["mb_win"].is_array() && extra["mb_win"].size() >= 2)
    {
        mb_win = extra["mb_win"].get< std::vector< double > >();
    }
    if(extra.contains("dr_win") && extra["dr_win"].is_array() && extra["dr_win"].size() >= 2)
    {
        dr_win = extra["dr_win"].get< std::vector< double > >();
    }
    if(extra.contains("ion_win") && extra["ion_win"].is_array() && extra["ion_win"].size() >= 2)
    {
        ion_win = extra["ion_win"].get< std::vector< double > >();
    }

    // Create text area for window table - positioned at bottom right, increased height
    auto text_ax = subplot2grid_wrapper(fSubplotConfig["window_textbox"]);

    text_ax->x_axis().visible(false);
    text_ax->y_axis().visible(false);
    text_ax->box(false);

    matplot::xlim({0, 1});
    matplot::ylim({0, 1});

    // Helper to format window values to 3 decimal places
    auto format_window_val = [](double val) -> std::string {
        std::ostringstream oss;
        oss << MHO_PlotDataExtractor::format_standard(val, 3, 3);
        return oss.str();
    };

    // Prepare table data
    std::vector< std::string > row_labels = {"sb window (us)", "mb window (us)", "dr window (ns/s)", "ion window (TEC)"};
    std::vector< std::pair< std::string, std::string > > window_ranges = {
        {sb_win.size() >= 2 ? format_window_val(sb_win[0]) : "0.000",
         sb_win.size() >= 2 ? format_window_val(sb_win[1]) : "0.000"  },
        {mb_win.size() >= 2 ? format_window_val(mb_win[0]) : "0.000",
         mb_win.size() >= 2 ? format_window_val(mb_win[1]) : "0.000"  },
        {dr_win.size() >= 2 ? format_window_val(dr_win[0]) : "0.000",
         dr_win.size() >= 2 ? format_window_val(dr_win[1]) : "0.000"  },
        {ion_win.size() >= 2 ? format_window_val(ion_win[0]) : "0.000",
         ion_win.size() >= 2 ? format_window_val(ion_win[1]) : "0.000"}
    };

    // Layout parameters - compact style
    double y_start = 1.0 - (0.006 + 0.09)*2;
    double y_step = 0.09; // Tight vertical spacing
    double label_x = 0.02;
    double min_val_x = 0.65;
    double max_val_x = 0.93;

    // Helper lambda for right-justified text with small font
    auto right_justify_text = [](matplot::axes_handle ax, double right_x, double y, const std::string& text) {
        double char_width = 0.006;
        double text_width = text.length() * char_width;
        double left_x = right_x - text_width;
        auto txt = ax->text(left_x, y, text);
        txt->font_size(8);
        return txt;
    };

    // Draw table rows
    for(size_t i = 0; i < row_labels.size(); ++i)
    {
        double y_pos = y_start - i * y_step;
        auto label = fLastAxis->text(label_x, y_pos, row_labels[i]);
        label->font_size(8);
        auto txt = fLastAxis->text(min_val_x, y_pos, window_ranges[i].first);
        txt->font_size(8);
        auto txt2 = fLastAxis->text(max_val_x, y_pos, window_ranges[i].second);
        txt2->font_size(8);
    }
}

void MHO_BasicPlotVisitor::make_data_stats_text(const mho_json& plot_dict)
{
    msg_debug("plot", "Adding data statistics text" << eom);

    if(!plot_dict.contains("extra"))
    {
        msg_warn("plot", "No extra data available for data statistics" << eom);
        return;
    }

    auto extra = plot_dict["extra"];

    // Extract data statistics
    int ambiguity = MHO_PlotDataExtractor::extract_double(extra, "ambiguity", 0);
    int ref_bits = MHO_PlotDataExtractor::extract_int(extra, "ref_station_sample_bits", 0);
    int rem_bits = MHO_PlotDataExtractor::extract_int(extra, "rem_station_sample_bits", 0);
    double sample_rate = MHO_PlotDataExtractor::extract_double(extra, "sample_rate", 0.0);
    int grid_pts = MHO_PlotDataExtractor::extract_int(extra, "grid_pts", 0);
    double data_rate = MHO_PlotDataExtractor::extract_double(extra, "data_rate", 0.0);
    int nlags = MHO_PlotDataExtractor::extract_int(extra, "nlags", 0);

    std::string ref_pc_mode = MHO_PlotDataExtractor::extract_string(extra, "ref_pc_mode", "");
    std::string rem_pc_mode = MHO_PlotDataExtractor::extract_string(extra, "rem_pc_mode", "");
    int ref_pc_period = MHO_PlotDataExtractor::extract_int(extra, "ref_pc_period", 0);
    int rem_pc_period = MHO_PlotDataExtractor::extract_int(extra, "rem_pc_period", 0);

    auto text_ax = subplot2grid_wrapper(fSubplotConfig["stats_textbox"]);

    text_ax->x_axis().visible(false);
    text_ax->y_axis().visible(false);
    text_ax->box(false);

    matplot::xlim({0, 1});
    matplot::ylim({0, 1});

    // Create multi-line data statistics text with smaller font size
    double y_pos = 0.9;
    double y_step = 0.095;

    auto text1 = fLastAxis->text(0.02, y_pos,
                               "Pcal mode: " + ref_pc_mode + ", " + rem_pc_mode + " PC period (AP's) " +
                                   std::to_string(ref_pc_period) + "," + std::to_string(rem_pc_period));
    text1->font_size(8); // Slightly smaller font
    y_pos -= y_step;

    auto text2 = fLastAxis->text(0.02, y_pos, "Pcal rate: X,X (us/s)"); //not implemented yet
    text2->font_size(8); // Slightly smaller font
    y_pos -= y_step;

    auto text3 = fLastAxis->text(0.02, y_pos,
                               "Bits/sample: " + std::to_string(ref_bits) + "x" + std::to_string(rem_bits) +
                                   "      SampCntNorm: disabled");
    text3->font_size(8); // Slightly smaller font
    y_pos -= y_step;

    auto text4 = fLastAxis->text(0.02, y_pos,
                               "Data rate(MSamp/s) " + std::to_string(static_cast< int >(sample_rate)) + " MBpts " +
                                   std::to_string(grid_pts) + " Amb " + MHO_PlotDataExtractor::format_standard(ambiguity,3,3) + " us");
    text4->font_size(8); // Slightly smaller font
    y_pos -= y_step;

    auto text5 = fLastAxis->text(0.02, y_pos,
                               "Data rate(Mb/s) " + std::to_string(static_cast< int >(data_rate)) +
                                   "  nlags: " + std::to_string(nlags) + "   t\\\\_cohere infinite");
    text5->font_size(8); // Slightly smaller font
}

void MHO_BasicPlotVisitor::make_channel_info_table(const mho_json& plot_dict)
{
    msg_debug("plot", "Creating channel info table" << eom);

    // Helper lambda to extract JSON data as string vector, handling both string and numeric types
    auto extract_as_string_vector = [](const mho_json& json_obj, const std::string& key) -> std::vector< std::string > {
        std::vector< std::string > result;
        try
        {
            if(!json_obj.contains(key))
            {
                return result;
            }

            const auto& data = json_obj[key];
            if(data.is_array())
            {
                for(const auto& item : data)
                {
                    if(item.is_string())
                    {
                        result.push_back(item.get< std::string >());
                    }
                    else if(item.is_number())
                    {
                        result.push_back(std::to_string(item.get< double >()));
                    }
                    else
                    {
                        result.push_back("?");
                    }
                }
            }
            else
            {
                msg_warn("plot", "Expected array for key '" << key << "' but got different type" << eom);
            }
        }
        catch(const std::exception& e)
        {
            msg_warn("plot", "Failed to extract data for key '" << key << "': " << e.what() << eom);
        }
        return result;
    };

    // Check if we have the required PLOT_INFO section
    if(!plot_dict.contains("PLOT_INFO"))
    {
        msg_warn("plot", "No PLOT_INFO section available for channel info table" << eom);
        return;
    }

    int n_plots = MHO_PlotDataExtractor::extract_int(plot_dict, "NPlots", 1);
    int n_channel_plots = (n_plots > 1) ? n_plots - 1 : n_plots; // Skip "All" channel

    if(n_channel_plots <= 0)
    {
        msg_warn("plot", "No channels to display in info table" << eom);
        return;
    }

    auto plot_info = plot_dict["PLOT_INFO"];

    // Check if we have header information
    if(!plot_info.contains("header"))
    {
        msg_warn("plot", "No header information in PLOT_INFO" << eom);
        return;
    }

    // Create text area aligned just below the channel plots 
    auto text_ax = subplot2grid({35, 1}, {23, 0}, 8, 1, fLeftMargin, fRightMargin);
    text_ax->x_axis().visible(false);
    text_ax->y_axis().visible(false);
    text_ax->box(false);

    matplot::xlim({0, 1});
    matplot::ylim({0, 1});

    // Extract headers
    std::vector< std::string > headers;
    try
    {
        headers = plot_info["header"].get< std::vector< std::string > >();
    }
    catch(const std::exception& e)
    {
        msg_warn("plot", "Failed to parse header information: " << e.what() << eom);
        return;
    }

    // Define header text mappings
    std::map< std::string, std::string > header_text = {
        {"#Ch",       ""              },
        {"Freq(MHz)", "Freq (MHz)"    },
        {"Phase",     "Phase"         },
        {"Ampl",      "Ampl."         },
        {"SbdBox",    "Sbd box"       },
        {"APsRf",     "APs used"      },
        {"APsRm",     "APs used"      },
        {"PCdlyRf",   "PC delays (ns)"},
        {"PCdlyRm",   "PC delays (ns)"},
        {"PCPhsRf",   "PC phase"      },
        {"PCPhsRm",   "PC phase"      },
        {"PCOffRf",   "Manl PC"       },
        {"PCOffRm",   "Manl PC"       },
        {"PCAmpRf",   "PC amp"        },
        {"PCAmpRm",   "PC amp"        },
        {"ChIdRf",    "Chan ids"      },
        {"TrkRf",     "Tracks"        },
        {"ChIdRm",    "Chan ids"      },
        {"TrkRm",     "Tracks"        }
    };

    // Check for polarization info to update PC delay headers
    if(plot_dict.contains("extra") && plot_dict["extra"].contains("pol_product"))
    {
        try
        {
            // Handle both string and array formats for pol_product
            if(plot_dict["extra"]["pol_product"].is_string())
            {
                std::string pol_str = plot_dict["extra"]["pol_product"].get< std::string >();
                if(pol_str.length() >= 2)
                {
                    header_text["PCdlyRf"] = "PC " + std::string(1, pol_str[0]) + " delays (ns)";
                    header_text["PCdlyRm"] = "PC " + std::string(1, pol_str[1]) + " delays (ns)";
                }
            }
            else if(plot_dict["extra"]["pol_product"].is_array())
            {
                auto pol_product = plot_dict["extra"]["pol_product"].get< std::vector< std::string > >();
                if(pol_product.size() >= 2)
                {
                    header_text["PCdlyRf"] = "PC " + pol_product[0] + " delays (ns)";
                    header_text["PCdlyRm"] = "PC " + pol_product[1] + " delays (ns)";
                }
            }
        }
        catch(const std::exception& e)
        {
            msg_warn("plot", "Failed to parse polarization info: " << e.what() << eom);
        }
    }

    // Define paired data relationships
    std::map< std::string, std::string > paired_data = {
        {"APsRf",   "APsRm"  },
        {"PCPhsRf", "PCPhsRm"},
        {"PCOffRf", "PCOffRm"}
    };

    // Reverse mapping for paired data
    std::set< std::string > paired_secondaries;
    for(const auto& pair : paired_data)
    {
        paired_secondaries.insert(pair.second);
    }

    // Process headers and create table data
    std::vector< std::string > table_headers;
    std::vector< std::vector< std::string > > table_data;

    for(const auto& header : headers)
    {
        // Skip if this is a secondary paired header (will be processed with primary)
        if(paired_secondaries.find(header) != paired_secondaries.end())
        {
            continue;
        }

        std::string header_display = header_text[header];
        if(header_display.empty())
        {
            continue; // Skip empty headers
        }

        // Check if this is a paired data header
        if(paired_data.find(header) != paired_data.end())
        {
            // Process paired data
            std::string secondary_header = paired_data[header];

            try
            {
                auto data1 = extract_as_string_vector(plot_info, header);
                auto data2 = extract_as_string_vector(plot_info, secondary_header);

                std::vector< std::string > paired_values;
                size_t min_size = std::min(data1.size(), data2.size());

                for(size_t i = 0; i < min_size && i < static_cast< size_t >(n_channel_plots); ++i)
                {
                    if(header == "APsRf")
                    {
                        // Format as "X/Y" for APs
                        int val1 = static_cast< int >(std::round(std::stod(data1[i])));
                        int val2 = static_cast< int >(std::round(std::stod(data2[i])));
                        paired_values.push_back(std::to_string(val1) + "/" + std::to_string(val2));
                    }
                    else
                    {
                        // Format as "X:Y" for other paired data
                        int val1 = static_cast< int >(std::round(std::stod(data1[i])));
                        int val2 = static_cast< int >(std::round(std::stod(data2[i])));
                        paired_values.push_back(std::to_string(val1) + ":" + std::to_string(val2));
                    }
                }

                // Add header name as last column
                paired_values.push_back(header_display);

                table_headers.push_back(header_display);
                table_data.push_back(paired_values);
            }
            catch(const std::exception& e)
            {
                msg_warn("plot", "Failed to process paired data for header '" << header << "': " << e.what() << eom);
            }
        }
        else
        {
            // Process single column data
            try
            {
                auto data = extract_as_string_vector(plot_info, header);
                std::vector< std::string > values;

                // Process only up to n_channel_plots (exclude "All" column)
                size_t data_size = std::min(data.size(), static_cast< size_t >(n_channel_plots));

                if(header_display == "Chan ids" || header_display == "Tracks")
                {
                    // Keep string fields as-is
                    for(size_t i = 0; i < data_size; ++i)
                    {
                        values.push_back(data[i]);
                    }
                }
                else
                {
                    // Format numeric fields to 1 decimal place
                    for(size_t i = 0; i < data_size; ++i)
                    {
                        try
                        {
                            double val = std::stod(data[i]);
                            std::ostringstream oss;
                            oss << std::fixed << std::setprecision(1) << val;
                            values.push_back(oss.str());
                        }
                        catch(const std::exception&)
                        {
                            values.push_back(data[i]); // Keep original if conversion fails
                        }
                    }
                }

                // Add header name as last column
                values.push_back(header_display);

                table_headers.push_back(header_display);
                table_data.push_back(values);
            }
            catch(const std::exception& e)
            {
                msg_warn("plot", "Failed to process data for header '" << header << "': " << e.what() << eom);
            }
        }
    }

    if(table_data.empty())
    {
        msg_warn("plot", "No valid table data to display" << eom);
        return;
    }

    // Calculate layout parameters
    double table_width = 1.0;
    double col_width = table_width / (n_channel_plots + 1); // +1 for header column
    double row_height = 1.0 / table_data.size() / 1.5; // Reduce row height by factor of 1.5
    int font_size = 7; //this might be somewhat conservative (on the small size)
    int n_decrement = std::floor(n_channel_plots/7 - 1);
    font_size -= n_decrement;
    if(font_size <= 2){font_size = 2;}

    // Draw the table with tighter row spacing
    for(size_t row = 0; row < table_data.size(); ++row)
    {
        double y_pos = 0.95 - row * row_height; // Start from top, no centering padding

        for(size_t col = 0; col < table_data[row].size(); ++col)
        {
            double x_pos = col * col_width;// + col_width * 0.1; // Small left padding

            // Update text to integer value - re-create the text object with proper color
            auto text = fLastAxis->text(x_pos, y_pos, table_data[row][col]);
            text->font_size(font_size);

            // Apply color coding for PC amplitude rows
            if(table_headers[row] == "PC amp" && col < static_cast< size_t >(n_channel_plots))
            {
                try
                {
                    int pc_amp = static_cast< int >(std::round(std::stod(table_data[row][col])));
                    std::string color;
                    if(pc_amp < 4 || pc_amp >= 150)
                    {
                        text->color("red");
                    }
                    else if(pc_amp < 10 || pc_amp >= 100)
                    {
                        text->color("#FFA500"); // HTML orange
                    }
                    else
                    {
                        text->color("#228B22"); //forest green
                    }
                }
                catch(const std::exception&)
                {
                    // Keep original text and color if conversion fails
                }
            }
        }
    }
}

} // namespace hops
