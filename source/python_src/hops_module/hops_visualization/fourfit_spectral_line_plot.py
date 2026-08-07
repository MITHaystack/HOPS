"""fourfit spectral-line fringe-plot renderer.

A variant of the default renderer for spectral-line data: it swaps the
delay-rate / single-band / cross-power panels for spectral-line panels
(``make_sl_*`` below) but reuses all of the shared panel/table/text builders
from :mod:`hops_visualization.fourfit_plot_common`. Keeping the shared helpers
in the common module means a layout change made for the default plot is picked
up here automatically instead of drifting.
"""

import time
import os, sys

# Shared builders + matplotlib setup (headless-safe Agg backend) from common.
from .fourfit_plot_common import *
from .fourfit_plot_common import (
    make_dr_mbd_plot, make_sbd_dtec_plot, make_xpower_plot,
    make_info_text_box, make_top_info_text, press_event_handler,
)

def make_sl_dr_spectrum_plot(plot_dict):
    '''1-D delay-rate spectrum for spectral-line data.
    Replaces make_dr_mbd_plot() when plot_dict["extra"]["is_spectral_line"] is True.
    Occupies the same grid cell as the broadband DR/MBD panel.
    '''
    dly_x = plot_dict.get('DLYRATE_XAXIS', np.arange(len(plot_dict['DLYRATE'])))
    dly_y = plot_dict['DLYRATE']

    ax1 = plt.subplot2grid((16,16),(0,0),rowspan=3,colspan=13)
    plt.subplots_adjust(left=0.07, right=0.93, bottom=0.07, top=0.93)

    ax1.plot(dly_x, dly_y, 'r-', linewidth=0.5)
    ax1.set_xlim(dly_x[0], dly_x[-1])
    ax1.set_ylim(bottom=0)
    ax1.set_xlabel('delay rate (ns/s)', fontsize=9)
    ax1.xaxis.label.set_color('r')
    ax1.xaxis.set_major_formatter(FormatStrFormatter('%.3f'))
    ax1.set_ylabel('amplitude', fontsize=9)
    ax1.yaxis.label.set_color('r')
    ax1.minorticks_on()
    ax1.tick_params(axis='both', direction='in', which='both', bottom=True, left=True)
    plt.xticks(fontsize=8)
    plt.yticks(fontsize=8, rotation=90)

    # Mark the fitted peak with a vertical line.
    if 'extra' in plot_dict and 'sl_peak_dr_ns_per_s' in plot_dict['extra']:
        peak_dr = float(plot_dict['extra']['sl_peak_dr_ns_per_s'])
        ax1.axvline(x=peak_dr, color='r', linestyle='--', linewidth=0.7, alpha=0.7)

    ax1.set_title('Delay-rate spectrum (spectral line)', fontsize=8, loc='left')

def make_sl_2d_surface_plot(plot_dict):
    '''2-D delay-rate x frequency amplitude surface for spectral-line data.
    Replaces make_sbd_dtec_plot() when plot_dict["extra"]["is_spectral_line"] is True.
    Occupies the same grid cell as the broadband SBD panel.
    '''
    dr_axis  = np.array(plot_dict.get('SL_2D_DR_AXIS',   []))
    freq_axis = np.array(plot_dict.get('SL_2D_FREQ_AXIS', []))
    amp_2d   = np.array(plot_dict.get('SL_2D_AMP',       []))  # shape (n_dr, n_freq)

    ax3 = plt.subplot2grid((16,16),(4,0),rowspan=2,colspan=6)

    if amp_2d.ndim == 2 and amp_2d.size > 0 and len(dr_axis) > 0 and len(freq_axis) > 0:
        # imshow with freq on x-axis, DR on y-axis; origin='lower' so DR increases upward.
        extent = [float(freq_axis[0]), float(freq_axis[-1]),
                  float(dr_axis[0]),   float(dr_axis[-1])]
        im = ax3.imshow(amp_2d, aspect='auto', origin='lower',
                        extent=extent, cmap='inferno', interpolation='nearest')

        ax3.set_xlabel('sky freq (MHz)', fontsize=9)
        ax3.set_ylabel('DR (ns/s)', fontsize=9)
        ax3.xaxis.set_major_formatter(FormatStrFormatter('%.2f'))
        ax3.yaxis.set_major_formatter(FormatStrFormatter('%.3f'))
        ax3.tick_params(axis='both', direction='in', which='both')
        plt.xticks(fontsize=8)
        plt.yticks(fontsize=8, rotation=90)

        # Mark the fitted peak with crosshairs.
        if 'extra' in plot_dict:
            ex = plot_dict['extra']
            if 'sl_peak_dr_ns_per_s' in ex and 'sl_peak_freq_axis_val' in ex:
                ax3.axvline(x=float(ex['sl_peak_freq_axis_val']), color='w',
                            linestyle='--', linewidth=0.6, alpha=0.8)
                ax3.axhline(y=float(ex['sl_peak_dr_ns_per_s']), color='w',
                            linestyle='--', linewidth=0.6, alpha=0.8)
    else:
        ax3.text(0.5, 0.5, 'no 2-D data', ha='center', va='center',
                 transform=ax3.transAxes, fontsize=8)
        ax3.axis('off')

    ax3.set_title('DR x freq surface', fontsize=8, loc='left')

def make_sl_freq_spectrum_plot(plot_dict):
    '''1-D frequency spectrum (amplitude + phase) at the fitted (channel, DR) bin.
    Replaces make_xpower_plot() when plot_dict["extra"]["is_spectral_line"] is True.
    Occupies the same grid cell as the broadband cross-power spectrum panel.
    '''
    freq_x = plot_dict.get('SL_FREQ_XAXIS', np.arange(len(plot_dict.get('SL_FREQ_AMP', []))))
    amp_y  = plot_dict.get('SL_FREQ_AMP',  [])
    phs_y  = plot_dict.get('SL_FREQ_PHS',  [])

    ax4 = plt.subplot2grid((16,16),(4,7),rowspan=2,colspan=6)

    ax4.plot(freq_x, amp_y, 'co-', markersize=2, markerfacecolor='b',
             linewidth=0.5, markeredgewidth=0.0)
    if len(freq_x) > 0:
        ax4.set_xlim(float(freq_x[0]), float(freq_x[-1]))
    ax4.set_ylim(bottom=0)
    ax4.set_xlabel('sky freq (MHz)', fontsize=9)
    ax4.xaxis.label.set_color('k')
    ax4.xaxis.set_major_formatter(FormatStrFormatter('%.2f'))
    ax4.set_ylabel('amplitude', fontsize=9)
    ax4.yaxis.label.set_color('b')
    ax4.minorticks_on()
    ax4.tick_params(axis='both', direction='in', which='both', right=True, bottom=True, left=True, top=True)
    plt.xticks(fontsize=8)
    plt.yticks(fontsize=8, rotation=90)

    # Phase on twin y-axis.
    ax5 = ax4.twinx()
    ax5.plot(freq_x, phs_y, 'ro', markersize=2, linewidth=0.5, markeredgewidth=0.0)
    if len(freq_x) > 0:
        ax5.set_xlim(float(freq_x[0]), float(freq_x[-1]))
    ax5.set_ylim(-180, 180)
    ax5.set_ylabel('phase [deg]', fontsize=9)
    ytick_locs = [-180, -90, 0, 90, 180]
    ax5.set_yticks(ytick_locs)
    ax5.set_yticklabels([str(y) for y in ytick_locs], fontsize=8, rotation=90)
    ax5.yaxis.label.set_color('r')
    ax5.tick_params(axis='both', direction='in', which='both')

    # Mark the fitted peak frequency with a vertical line.
    if 'extra' in plot_dict and 'sl_peak_freq_axis_val' in plot_dict['extra']:
        peak_fq = float(plot_dict['extra']['sl_peak_freq_axis_val'])
        ax4.axvline(x=peak_fq, color='b', linestyle='--', linewidth=0.7, alpha=0.7)

    ax4.set_title('Sky freq spectrum at peak DR bin', fontsize=8, loc='left')

def make_sl_phasor_time_plot(plot_dict):
    '''Peak-phasor amplitude and phase vs time (delay-rate correction only).
    Placed in the same grid cell as make_channel_segment_plots_alt.
    Data: SL_SEG_AMP (amplitude per segment), SL_SEG_PHS (phase in degrees per segment).
    '''
    sl_seg_amp = plot_dict.get('SL_SEG_AMP', [])
    sl_seg_phs = plot_dict.get('SL_SEG_PHS', [])
    n_seg = len(sl_seg_amp)

    ax6 = plt.subplot2grid((16,16),(7,0),rowspan=3,colspan=13)

    if n_seg == 0:
        ax6.text(0.5, 0.5, 'no segment data', ha='center', va='center',
                 transform=ax6.transAxes, fontsize=8)
        ax6.axis('off')
        return

    seg_x = np.arange(n_seg)
    ax6.plot(seg_x, sl_seg_amp, 'co-', markersize=2, markerfacecolor='b',
             linewidth=0.5, markeredgewidth=0.0)
    ax6.set_xlim(0, max(n_seg - 1, 1))
    ax6.set_ylim(bottom=0)
    ax6.set_xlabel('time segment', fontsize=9)
    ax6.set_ylabel('amplitude', fontsize=9)
    ax6.yaxis.label.set_color('b')
    ax6.minorticks_on()
    ax6.tick_params(axis='both', direction='in', which='both')
    plt.xticks(fontsize=8)
    plt.yticks(fontsize=8, rotation=90)

    ax6a = ax6.twinx()
    ax6a.plot(seg_x, sl_seg_phs, 'ro', markersize=2, linewidth=0.5, markeredgewidth=0.0)
    ax6a.set_xlim(0, max(n_seg - 1, 1))
    ax6a.set_ylim(-180, 180)
    ax6a.set_ylabel('phase [deg]', fontsize=9)
    ytick_locs = [-180, -90, 0, 90, 180]
    ax6a.set_yticks(ytick_locs)
    ax6a.set_yticklabels([str(y) for y in ytick_locs], fontsize=8, rotation=90)
    ax6a.yaxis.label.set_color('r')
    ax6a.tick_params(axis='both', direction='in', which='both')

    ax6.set_title('Peak phasor vs time (DR-corrected)', fontsize=8, loc='left')

def make_fourfit_spectral_line_plot(plot_dict, show_on_screen, filename):
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

    # Build the figure.  We'll construct this figure using many subplots, with different grid specifications.
    fig = pylab.figure(1)

    # Detect spectral-line data and dispatch to the appropriate panel functions.
    is_sl = plot_dict.get('extra', {}).get('is_spectral_line', False)

    t1 = time.process_time()
    if is_sl:
        make_sl_dr_spectrum_plot(plot_dict)    # 1-D DR spectrum
        make_sl_2d_surface_plot(plot_dict)     # 2-D DR x freq surface
        make_sl_freq_spectrum_plot(plot_dict)  # 1-D freq spectrum (amp + phase)
        make_sl_phasor_time_plot(plot_dict)    # peak phasor amp/phase vs time
    else:
        make_dr_mbd_plot(plot_dict)    #constructs the delay-rate/multiband delay twin plot
        make_sbd_dtec_plot(plot_dict)  #constructs the single-band delay and (ion-dTEC) twin plot
        make_xpower_plot(plot_dict)    #constructs the cross-power spectrum phase/amp twin plot
    t2 = time.process_time()
    #print("time for first few plots: ", t2 - t1)  #takes like 0.3 sec

    #THESE PLOTS ARE SUPER SLOW
    #t1 = time.process_time()
    # make_channel_segment_plots_alt(fig, plot_dict) #constructs the per-channel phase/amp plots
    # make_channel_segment_validity_plots(fig, plot_dict) #constructs the USB/LSB validity flags
    # make_pcal_plots(fig, plot_dict) #constructs the per-channel p-cal plots
    # make_channel_info_table(plot_dict) #constructs the channel/pcal info table
    #t2 = time.process_time()
    #print("time for slow functions: ", t2 - t1) #takes like 5.5 sec

    t1 = time.process_time()
    make_info_text_box(plot_dict) #constructs fringe summary text box
    make_top_info_text(plot_dict) #constructs the title/top-page info
    # make_model_resid_info_text(plot_dict) #constructs the a priori model, totals, and residuals text at the bottom
    # make_rms_table(plot_dict) #constructs the fringe RMS table
    # make_coord_text(plot_dict) #constructs the station coordinate statements (az,el,pa,u,v)
    # make_amplitude_table(plot_dict) #constructs the amplitude table
    # make_window_table(plot_dict) #constructs the (sbd,mbd,dr,ion) window limits table
    # make_data_stats_text(plot_dict) #constructs the data statistics/summary text
    t2 = time.process_time()
    #print("time for rest of text functions: ", t2 - t1) #takes like .05 sec

    if filename != "":
        pylab.savefig(filename)

    if show_on_screen:
        #handler to capture key presses to exit plot and continue
        fig.canvas.mpl_connect('key_press_event', press_event_handler)
        pylab.show() #blocking

    plt.close('all')
    fig.canvas.flush_events()

def make_fourfit_spectral_line_plot_wrapper(fringe_data_interface):
    plot_file = "";
    show_plot = False
    if fringe_data_interface.get_parameter_store().is_present("/cmdline/disk_file") is True:
        plot_file = fringe_data_interface.get_parameter_store().get_by_path("/cmdline/disk_file");
    if fringe_data_interface.get_parameter_store().is_present("/cmdline/show_plot") is True:
        show_plot = fringe_data_interface.get_parameter_store().get_by_path("/cmdline/show_plot");

    make_fourfit_spectral_line_plot(fringe_data_interface.get_plot_data(), show_plot, plot_file)
