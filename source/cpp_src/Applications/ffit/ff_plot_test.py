
import os, sys

import numpy as np

import matplotlib
#matplotlib.use("Agg", warn=False)

import matplotlib.pyplot as plt
import matplotlib.gridspec as gridspec
matplotlib.rcParams.update({'savefig.dpi':300})
import pylab
from matplotlib.ticker import FormatStrFormatter


def fourfit_plot(plot_dict, filename):
    '''
    Function to reproduce a fourfit fringe plot.

    Parameters
    ----------

    plot_dict : dict
        Dictionary with key/value pairs for the plot.

    filename : str
        Path of the filename to save the plot.


    Returns
    -------

    None

    '''

    matplotlib.rcParams.update({'figure.figsize': [8.5,11]})

    # build x-axis arrays for SBD, MBD, delayrate, Xpow spec, etc

    sbd_x = plot_dict['SBD_AMP_XAXIS']
    mbd_x = plot_dict['MBD_AMP_XAXIS']
    dly_x = plot_dict['DLYRATE_XAXIS']
    xpow_x = plot_dict['XPSPEC_XAXIS']
    # xpow_x = np.arange(-2,2,4.0/len(plot_dict['XPSPEC-ABS']))


    # Build the figure.  We'll construct this figure using many subplots, with different grid specifications.

    fig = pylab.figure(1)

    # The delay rate panel (shares axes with the multiband delay)
    ax1 = plt.subplot2grid((16,16),(0,0),rowspan=4,colspan=13)
    ax1.plot(dly_x, plot_dict['DLYRATE'],'r-',linewidth=0.5)
    ax1.set_xlim(dly_x[0],dly_x[-1])
    ax1.set_ylim(bottom=0)
    ax1.set_xlabel('delay rate (ns/s)',fontsize=9)
    plt.xticks(fontsize=8)
    ax1.xaxis.label.set_color('r')
    ax1.xaxis.set_major_formatter(FormatStrFormatter('%.3f'))
    ax1.set_ylabel('amplitude',fontsize=9)
    plt.yticks(fontsize=8,rotation=90)
    ax1.yaxis.label.set_color('r')
    ax1.minorticks_on()
    ax1.tick_params(axis='both', direction='in', which='both')
    #
    # twin both axes, retain the intermediate axes object to adjust those ticks
    ax2a = ax1.twinx()
    plt.yticks(fontsize=8,rotation=90)
    ax2 = ax2a.twiny()
    ax2.plot(mbd_x, plot_dict['MBD_AMP'],'b-',linewidth=0.5)
    ax2.set_xlim(mbd_x[0],mbd_x[-1])
    ax2.set_ylim(bottom=0)
    ax2.set_xlabel(r'multiband delay ($\mu$s)',fontsize=9)
    ax2.xaxis.set_major_formatter(FormatStrFormatter('%.3f'))
    plt.xticks(fontsize=8)
    ax2.xaxis.label.set_color('b')
    ax2.minorticks_on()
    ax2.tick_params(axis='both', direction='in', which='both')

    # ax2a.set_ylabel('amplitude',fontsize=9)
    # ax2a.yaxis.label.set_color('b')



    # The singleband delay panel
    ax3 = plt.subplot2grid((16,8),(5,0),rowspan=3,colspan=3)

    ax3.plot(sbd_x, plot_dict['SBD_AMP'],'g-',linewidth=0.5)
    ax3.set_xlim(sbd_x[0],sbd_x[-1])
    ax3.set_ylim(bottom=0)
    ax3.set_xlabel(r'singleband delay ($\mu$s)',fontsize=9)
    plt.xticks(fontsize=8)
    ax3.xaxis.label.set_color('g')
    ax3.xaxis.set_major_formatter(FormatStrFormatter('%.1f'))
    ax3.set_ylabel('amplitude',fontsize=9)
    plt.yticks(fontsize=8,rotation=90)
    ax3.yaxis.label.set_color('g')
    ax3.minorticks_on()
    ax3.tick_params(axis='both', direction='in', which='both')

    # The cross-power sepctra
    ax4 = plt.subplot2grid((16,16),(5,7),rowspan=3,colspan=6)
    ax4.plot(xpow_x, plot_dict['XPSPEC-ABS'],'co-',markersize=2,markerfacecolor='b',linewidth=0.5, markeredgewidth=0.0)
    ax4.set_xlim(xpow_x[0],xpow_x[-1])
    ax4.set_ylim(bottom=0)
    ax4.set_xlabel('Avgd XPow Spectrum (MHz)',fontsize=9)
    plt.xticks(fontsize=8)
    ax4.xaxis.label.set_color('k')
    ax4.xaxis.set_major_formatter(FormatStrFormatter('%.1f'))
    ax4.set_ylabel('amplitude',fontsize=9)
    plt.yticks(fontsize=8,rotation=90)
    ax4.yaxis.label.set_color('b')
    ax4.minorticks_on()
    ax4.tick_params(axis='both', direction='in', which='both')

    ax5 = ax4.twinx()
    ax5.plot(xpow_x, plot_dict['XPSPEC-ARG'],'ro',markersize=2,linewidth=0.5, markeredgewidth=0.0)
    ax5.set_xlim(xpow_x[0],xpow_x[-1])
    ax5.set_ylim(-180,180)
    ax5.set_ylabel('phase [deg]',fontsize=9)
    ytick_locs = [-180,-90,0,90,180]
    ytick_labels = [str(yy) for yy in ytick_locs]
    plt.yticks(ytick_locs, ytick_labels, fontsize=8,rotation=90)
    ax5.yaxis.label.set_color('r')
    ax5.tick_params(axis='both', direction='in', which='both')

    # Now we build the plots for each band; this will need attention, currently only supports six bands
    # (and is built by hand; need to loop)

    n_seg = plot_dict["NSeg"]
    n_seg_plots = plot_dict["NPlots"]
    colw = 6

    #grab seg amp
    seg_amp_arr = np.array( plot_dict['SEG_AMP'] )
    seg_ymax = float(plot_dict['Amp'])*3.0 #see generate_graphs.c

    # convert SEG_PHS to deg
    seg_phs_arr = np.array( [xx*180/np.pi for xx in plot_dict['SEG_PHS']] )

    seg_amp_arr1 = seg_amp_arr.reshape(n_seg_plots,n_seg)
    seg_phs_arr1 = seg_phs_arr.reshape(n_seg_plots,n_seg)

    for ch in range(0,n_seg_plots):
        ax6 = plt.subplot2grid((16,colw*n_seg_plots),(9,colw*ch),rowspan=3,colspan=colw)
        plt.subplots_adjust(wspace=0, hspace=0)
        ax6.plot(range(n_seg), seg_amp_arr1[ch],'co-',markersize=2, markerfacecolor='b', linewidth=0.5, markeredgewidth=0.0)
        ax6.set_xlim(0,n_seg)
        ax6.set_ylim(0,seg_ymax)
        ax6.set_xticklabels(labels=[],visible=False)
        ax6.tick_params(axis='both', direction='in', which='both')
        if ch == 0:
            ax6.set_ylabel('amplitude',fontsize=9)
            plt.yticks(fontsize=8,rotation=90)
            ax6.yaxis.label.set_color('b')
            ax6.minorticks_on()
            plt.tick_params(left = True, bottom = False)
        else:
            #ax6.axis("off")
            ax6.xaxis.set_major_locator(plt.NullLocator())
            ax6.yaxis.set_major_locator(plt.NullLocator())
            ax6.set_yticklabels(labels=[],visible=False)
            plt.yticks(visible=False)
            plt.tick_params(left = False, bottom = False)

        ax6a = ax6.twinx()
        ax6a.plot(range(n_seg), seg_phs_arr1[ch],'ro',markersize=2, linewidth=0.5, markeredgewidth=0.0)
        ax6a.set_xlim(0,n_seg)
        ax6a.set_ylim(-180,180)
        ax6a.tick_params(axis='both', direction='in', which='both')
        if ch == n_seg_plots-1:
            ax6a.set_ylabel('phase [deg]',fontsize=9)
            ax6a.set_ylabel('phase [deg]',fontsize=9)
            ytick_locs = [-180,-90,0,90,180]
            ytick_labels = [str(yy) for yy in ytick_locs]
            plt.yticks(ytick_locs, ytick_labels, fontsize=8,rotation=90)
            ax6a.yaxis.label.set_color('r')
            plt.tick_params(right = True, bottom = False)
        else:
            #ax6a.axis("off")
            ax6a.xaxis.set_major_locator(plt.NullLocator())
            ax6a.yaxis.set_major_locator(plt.NullLocator())
            ax6a.set_yticklabels(labels=[],visible=False)
            plt.tick_params(left = False, bottom = False)
        # ax6.plot(range(n_seg), seg_amp_arr1[ch],'co-',markersize=2, markerfacecolor='b', linewidth=0.5, markeredgewidth=0.0)
        # ax6a.plot(range(n_seg), seg_phs_arr1[ch],'ro',markersize=2, linewidth=0.5, markeredgewidth=0.0)

    # # Now build the text boxes
    #
    props = dict(boxstyle='square',facecolor='white',alpha=0.5)
    textstr1 = 'Fringe quality ' + '\n\n' + \
        'SNR ' + '\n' + \
        'Int time ' + '\n' + \
        'Amp ' + '\n' + \
        'Phase ' + '\n' + \
        'PFD ' + '\n' + \
        'Delay (us) ' + '\n' + \
        'SBD ' + '\n' + \
        'MBD ' + '\n' + \
        'Fringe rate (Hz) ' + '\n\n' + \
        'Ion TEC ' + '\n' + \
        'Ref freq (MHz) ' + '\n\n' + \
        'AP (sec) ' + '\n' + \
        'Exp. ' + '\n' + \
        'Exper # ' + '\n' + \
        'Yr:day ' + '\n' + \
        'Start ' + '\n' + \
        'Stop ' + '\n' + \
        'FRT ' + '\n' + \
        'Com/FF/build ' + '\n\n\n\n' + \
        'RA & Dec (J2000)'



    textstr2 = str(plot_dict['Quality']) + '\n\n' + \
        str(np.round(float(plot_dict['SNR']),1)) + '\n' + \
        str(np.round(float(plot_dict['IntgTime']),3)) + '\n' + \
        str(np.round(float(plot_dict['Amp']),3)) + '\n' + \
        str(np.round(float(plot_dict['ResPhase']),1)) + '\n' + \
        str(np.round(float(plot_dict['PFD']),2)) + '\n' + \
        '\n' + \
        str(np.round(float(plot_dict['ResidSbd(us)']),6)) + '\n' + \
        str(np.round(float(plot_dict['ResidMbd(us)']),6))  + '\n' + \
        '\n' + \
        str(np.round(float(plot_dict['FringeRate(Hz)']),6)) + '\n' + \
        plot_dict['IonTEC(TEC)'] + '\n\n' + \
        str(np.round(float(plot_dict['RefFreq(MHz)']),4)) + '\n' + \
        str(np.round(float(plot_dict['AP(sec)']),3)) + '\n' + \
        plot_dict['ExperName'] + '\n' + \
        str( plot_dict['ExperNum'] ) + '\n' + \
        plot_dict['YearDOY'] + '\n' + \
        plot_dict['Start'] + '\n' + \
        plot_dict['Stop'] + '\n' + \
        plot_dict['FRT'] + '\n\n' + \
        plot_dict['CorrTime'] + '\n' + \
        plot_dict['FFTime'] + '\n' + \
        plot_dict['BuildTime'] + '\n\n' + \
        plot_dict['RA'] + '\n' + \
        plot_dict['Dec']


    # Add the text boxes
    plt.text(0.805,0.9,textstr1,transform=plt.gcf().transFigure,fontsize=10,verticalalignment='top',
             family='monospace',horizontalalignment='left',color='g')

    plt.text(0.965,0.9,textstr2,transform=plt.gcf().transFigure,fontsize=10,verticalalignment='top',
             family='monospace',horizontalalignment='right',color='k')


    # Add the top matter
    plt.text(0.965,0.94,plot_dict['RootScanBaseline'].strip("'"),transform=plt.gcf().transFigure,
             fontsize=12,verticalalignment='top',family='sans-serif',horizontalalignment='right',fontweight='bold')

    plt.text(0.05,0.94,plot_dict['CorrVers'].strip("'"),transform=plt.gcf().transFigure,
             fontsize=12,verticalalignment='top',family='sans-serif',horizontalalignment='left',fontweight='bold')

    plt.text(0.965,0.92, plot_dict['PolStr'].strip("'"),transform=plt.gcf().transFigure,
             fontsize=10,verticalalignment='top',family='sans-serif',horizontalalignment='right',fontweight='bold')


    btmtextstr1 = 'Group delay (usec) ' + '\n' + \
        'Sband delay (usec) ' + '\n' + \
        'Phase delay (usec) ' + '\n' + \
        'Delay rate (us/s) ' + '\n' + \
        'Total Phase (deg) ' + '\n'

    group_delay_key = "GroupDelayModel(usec)"
    if not group_delay_key in plot_dict:
        group_delay_key = "GroupDelaySBD(usec)"


    btmtextstr2 = str(np.format_float_scientific( float(plot_dict[group_delay_key]), precision=11, min_digits=11 ) )  + '\n' + \
        str(np.format_float_scientific(float(plot_dict["SbandDelay(usec)"]), precision=11, min_digits=11) ) + '\n' + \
        str(np.format_float_scientific(float(plot_dict["PhaseDelay(usec)"]), precision=11, min_digits=11) ) + '\n' + \
        str(np.format_float_scientific(float(plot_dict["DelayRate(ps/s)"]), precision=11, min_digits=11) ) + '\n' + \
        str( np.round(float(plot_dict["TotalPhase(deg)"]),1) )

    btmtextstr3 = 'Apriori delay (usec) ' + '\n' + \
        'Apriori clock (usec) ' + '\n' + \
        'Apriori clockrate (us/s) ' + '\n' + \
        'Apriori rate (us/s) ' + '\n' + \
        'Apriori accel (us/s/s) ' + '\n'

    btmtextstr4 = str(np.format_float_scientific( float(plot_dict["AprioriDelay(usec)"]), precision=12, min_digits=11 ) )  + '\n' + \
        str(np.format_float_scientific(float(plot_dict["AprioriClock(usec)"]), precision=7, min_digits=7) ) + '\n' + \
        str(np.format_float_scientific(float(plot_dict["AprioriClockrate(us/s)"]), precision=7, min_digits=7) ) + '\n' + \
        str(np.format_float_scientific(float(plot_dict["AprioriRate(us/s)"]), precision=11, min_digits=11) ) + '\n' + \
        str(np.format_float_scientific(float(plot_dict["AprioriAccel(us/s/s)"]), precision=11, min_digits=11) )

    btmtextstr5 = 'Resid mbdelay (usec) ' + '\n' + \
        'Resid sbdelay (usec) ' + '\n' + \
        'Resid phdelay (usec) ' + '\n' + \
        'Resid rate (us/s) ' + '\n' + \
        'Resid phase (deg) ' + '\n'

    btmtextstr6 = str(np.format_float_scientific( float(plot_dict['ResidMbd(us)']), precision=5, min_digits=5 ) )  + '\n' + \
        str(np.format_float_scientific(float(plot_dict['ResidSbd(us)']), precision=5, min_digits=5) ) + '\n' + \
        str(np.format_float_scientific(float(plot_dict["ResidPhdelay(usec)"]), precision=5, min_digits=5) ) + '\n' + \
        str(np.format_float_scientific(float(plot_dict["ResidRate(us/s)"]), precision=5, min_digits=5) ) + '\n' + \
        str( np.round(float(plot_dict["ResidPhase(deg)"]),1) )

    btmtextstr7 = '+/-' + '\n' + \
        '+/-' + '\n' + \
        '+/-' + '\n' + \
        '+/-' + '\n' + \
        '+/-' + '\n'

    btmtextstr8 = str(np.format_float_scientific( float(plot_dict["ResidMbdelayError(usec)"]), precision=1, min_digits=1 ) )  + '\n' + \
        str(np.format_float_scientific(float(plot_dict["ResidSbdelayError(usec)"]), precision=1, min_digits=1) ) + '\n' + \
        str(np.format_float_scientific(float(plot_dict["ResidPhdelayError(usec)"]), precision=1, min_digits=1) ) + '\n' + \
        str(np.format_float_scientific(float(plot_dict["ResidRateError(us/s)"]), precision=1, min_digits=1) ) + '\n' + \
        str( np.round(float(plot_dict["ResidPhaseError(deg)"]),2) )

    # Add the text boxes
    plt.text(0.01,0.2,btmtextstr1,transform=plt.gcf().transFigure,fontsize=7,verticalalignment='top',
             family='monospace',horizontalalignment='left',color='k')

    # Add the text boxes
    plt.text(0.28,0.2,btmtextstr2,transform=plt.gcf().transFigure,fontsize=7,verticalalignment='top',
             family='monospace',horizontalalignment='right',color='k')

    plt.text(0.3,0.2,btmtextstr3,transform=plt.gcf().transFigure,fontsize=7,verticalalignment='top',
             family='monospace',horizontalalignment='left',color='k')

    plt.text(0.6,0.2,btmtextstr4,transform=plt.gcf().transFigure,fontsize=7,verticalalignment='top',
             family='monospace',horizontalalignment='right',color='k')

    plt.text(0.65,0.2,btmtextstr5,transform=plt.gcf().transFigure,fontsize=7,verticalalignment='top',
             family='monospace',horizontalalignment='left',color='k')

    plt.text(0.88,0.2,btmtextstr6,transform=plt.gcf().transFigure,fontsize=7,verticalalignment='top',
             family='monospace',horizontalalignment='right',color='k')

    plt.text(0.89,0.2,btmtextstr7,transform=plt.gcf().transFigure,fontsize=7,verticalalignment='top',
             family='monospace',horizontalalignment='left',color='k')

    plt.text(0.97,0.2,btmtextstr8,transform=plt.gcf().transFigure,fontsize=7,verticalalignment='top',
             family='monospace',horizontalalignment='right',color='k')











    #pylab.show()
    pylab.savefig(filename)
