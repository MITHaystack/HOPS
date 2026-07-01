"""fourfit default fringe-plot renderer.

Reproduces the standard fourfit fringe plot from the ``plot_dict`` built by the
C++ side. The panel/table/text-box builders live in
:mod:`hops_visualization.fourfit_plot_common` and are shared with the
spectral-line renderer; only the top-level assembly, the pybind wrapper, and the
no-embed subprocess entry point live here.
"""

import time
import os, sys

# Shared panel/table/text builders (and the matplotlib setup, incl. the
# headless-safe Agg backend) come from the common module.
from .fourfit_plot_common import *
from .fourfit_plot_common import (
    make_dr_mbd_plot, make_sbd_dtec_plot, make_xpower_plot,
    make_channel_segment_plots_alt, make_channel_segment_validity_plots,
    make_pcal_plots, make_channel_info_table, make_info_text_box,
    make_top_info_text, make_model_resid_info_text, make_rms_table,
    make_coord_text, make_amplitude_table, make_window_table,
    make_data_stats_text, press_event_handler,
)

def make_fourfit_plot(plot_dict, show_on_screen, filename):
    '''
    Function to reproduce a fourfit fringe plot.

    Parameters
    ----------
    plot_dict : dict
        Dictionary with key/value pairs for the plot.
    show_on_screen : bool
        Show the plot on-screen.
    filename : str
        Path of the filename to save the plot, if empty, plot will not be saved.

    Returns
    -------
    None
    '''

    matplotlib.rcParams.update({'figure.figsize': [8.5,11]})

    # pick the backend depending on display on/off (Agg/TkAgg),
    # do not use GTK4 (its object model conflicts with pybind11)
    if show_on_screen:
        plt.switch_backend("TkAgg")
    else:
        plt.switch_backend("Agg")

    # Build the figure.  We'll construct this figure using many subplots, with different grid specifications.
    fig = pylab.figure(1)

    t1 = time.process_time()
    make_dr_mbd_plot(plot_dict) #constructs the delay-rate/multiband delay twin plot
    make_sbd_dtec_plot(plot_dict) #constructs the single-band delay and (ion-dTEC) twin plot
    make_xpower_plot(plot_dict) #constructs the cross-power spectrum phase/amp twin plot
    t2 = time.process_time()
    #print("time for first few plots: ", t2 - t1)  #takes like 0.3 sec

    #THESE PLOTS ARE SUPER SLOW
    t1 = time.process_time()
    make_channel_segment_plots_alt(fig, plot_dict) #constructs the per-channel phase/amp plots
    make_channel_segment_validity_plots(fig, plot_dict) #constructs the USB/LSB validity flags
    make_pcal_plots(fig, plot_dict) #constructs the per-channel p-cal plots
    make_channel_info_table(plot_dict) #constructs the channel/pcal info table
    t2 = time.process_time()
    #print("time for slow functions: ", t2 - t1) #takes like 5.5 sec

    t1 = time.process_time()
    make_info_text_box(plot_dict) #constructs fringe summary text box
    make_top_info_text(plot_dict) #constructs the title/top-page info
    make_model_resid_info_text(plot_dict) #constructs the a priori model, totals, and residuals text at the bottom
    make_rms_table(plot_dict) #constructs the fringe RMS table
    make_coord_text(plot_dict) #constructs the station coordinate statements (az,el,pa,u,v)
    make_amplitude_table(plot_dict) #constructs the amplitude table
    make_window_table(plot_dict) #constructs the (sbd,mbd,dr,ion) window limits table
    make_data_stats_text(plot_dict) #constructs the data statistics/summary text
    t2 = time.process_time()
    #print("time for rest of text functions: ", t2 - t1) #takes like .05 sec

    if filename != "":
        pylab.savefig(filename)

    if show_on_screen:
        #handler to capture key presses to exit plot and continue
        fig.canvas.mpl_connect('key_press_event', press_event_handler)
        pylab.show() #blocking

    fig.canvas.flush_events()
    plt.close('all')

def make_fourfit_plot_wrapper(fringe_data_interface):
    plot_file = "";
    show_plot = False
    if fringe_data_interface.get_parameter_store().is_present("/cmdline/disk_file") is True:
        plot_file = fringe_data_interface.get_parameter_store().get_by_path("/cmdline/disk_file");
    if fringe_data_interface.get_parameter_store().is_present("/cmdline/show_plot") is True:
        show_plot = fringe_data_interface.get_parameter_store().get_by_path("/cmdline/show_plot");

    make_fourfit_plot(fringe_data_interface.get_plot_data(), show_plot, plot_file)

def _run_subprocess_entry(argv):
    '''
    Subprocess (no-embed) plotting entry point.

    Invoked by MHO_SubprocessPythonPlotVisitor as::

        python -m hops_visualization.fourfit_plot <request_file> <response_file>

    This is the Python half of the no-embed default-plot backend. The C++ side
    pre-creates both temp files: it writes the request into the first and reads
    the JSON response back from the second. It reads the JSON request, renders
    with make_fourfit_plot() (the same dict-driven renderer the embedded path
    uses), and writes a JSON response into the response file. Returning the
    contract through its own file (rather than stdout) means matplotlib / user
    plot chatter cannot corrupt what the C++ side parses.

    JSON contract (keep in sync with MHO_PythonSubprocessContract.hh):
        request  : { schema_version, plot_dict, show_plot, disk_file }
        response : { schema_version, ok, output_file, error }
    '''
    import json
    schema_version = 2

    if len(argv) < 3:
        sys.stderr.write(
            "fourfit_plot: usage: python -m hops_visualization.fourfit_plot <request_file> <response_file>\n"
        )
        return 2
    request_path = argv[1]
    response_path = argv[2]

    def emit(response):
        # Write the JSON contract into the response file the C++ caller reads;
        # keeping it off stdout means matplotlib/user chatter cannot corrupt it.
        with open(response_path, "w") as f:
            json.dump(response, f)

    try:
        if request_path == "-":
            request = json.load(sys.stdin)
        else:
            with open(request_path, "r") as f:
                request = json.load(f)

        req_version = request.get("schema_version")
        if req_version != schema_version:
            raise ValueError("plot request schema_version %r != expected %d" % (req_version, schema_version))

        plot_dict = request.get("plot_dict")
        if plot_dict is None:
            raise ValueError("request is missing 'plot_dict'")
        show_plot = bool(request.get("show_plot", False))
        filename = request.get("disk_file", "") or ""

        # make_fourfit_plot() selects the backend itself (headless-safe Agg when
        # not showing on-screen, TkAgg otherwise), so no backend switch here.
        make_fourfit_plot(plot_dict, show_plot, filename)

        emit({"schema_version": schema_version, "ok": True, "output_file": filename})
        return 0
    except Exception as exc:  # surface any failure as a contract error
        emit({"schema_version": schema_version, "ok": False, "error": "%s" % exc})
        return 1

if __name__ == "__main__":
    sys.exit(_run_subprocess_entry(sys.argv))
