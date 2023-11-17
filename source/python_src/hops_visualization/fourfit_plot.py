
import os, sys

import numpy as np

import matplotlib
#matplotlib.use("Agg", warn=False)

import matplotlib.pyplot as plt
import matplotlib.gridspec as gridspec
matplotlib.rcParams.update({'savefig.dpi':300})
import pylab
from matplotlib.ticker import FormatStrFormatter
from matplotlib.ticker import AutoMinorLocator

def make_fourfit_plot(plot_dict, filename):
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
    # build x-axis arrays for SBD, MBD, delayrate, Xpow spec, etc
    sbd_x = []
    mbd_x = []
    dly_x = []
    xpow_x = []

    # build x-axis arrays for SBD, MBD, delayrate, Xpow spec, etc
    if 'SBD_AMP_XAXIS' in plot_dict:
        sbd_x = plot_dict['SBD_AMP_XAXIS']
    else:
        sbd_x = np.arange(0,len(plot_dict['SBD_AMP']))

    if 'MBD_AMP_XAXIS' in plot_dict:
        mbd_x = plot_dict['MBD_AMP_XAXIS']
    else:
        mbd_x = np.arange(0,len(plot_dict['MBD_AMP']))

    if 'DLYRATE_XAXIS' in plot_dict:
        dly_x = plot_dict['DLYRATE_XAXIS']
    else:
        dly_x = np.arange(0,len(plot_dict['DLYRATE']))

    if 'XPSPEC_XAXIS' in plot_dict:
        xpow_x = plot_dict['XPSPEC_XAXIS']
    else:
        xpow_x = np.arange(0,len(plot_dict['XPSPEC-ABS'])/2)

    # xpow_x = np.arange(-2,2,4.0/len(plot_dict['XPSPEC-ABS']))


    # Build the figure.  We'll construct this figure using many subplots, with different grid specifications.

    fig = pylab.figure(1)

    # The delay rate panel (shares axes with the multiband delay)
    ax1 = plt.subplot2grid((16,16),(0,0),rowspan=3,colspan=13)
    plt.subplots_adjust(left=0.07, right=0.93, bottom=0.07, top=0.93)

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
    ax1.tick_params(axis='both', direction='in', which='both', bottom=True, left=True)

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
    ax2.tick_params(axis='both', direction='in', which='both', top=True, right=True)
    # ax2a.set_ylabel('amplitude',fontsize=9)
    # ax2a.yaxis.label.set_color('b')



    # The singleband delay panel
    ax3 = plt.subplot2grid((16, 16),(4,0),rowspan=2,colspan=6)
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
    ax3.tick_params(axis='both', direction='in', which='both', right=True, bottom=True, left=True, top=True)

    # The cross-power sepctra
    #ax4 = plt.subplot2grid((16,16),(5,7),rowspan=3,colspan=6)
    ax4 = plt.subplot2grid((16, 16),(4,7),rowspan=2,colspan=6)
    ax4.plot(xpow_x, plot_dict['XPSPEC-ABS'][0:len(xpow_x)],'co-',markersize=2,markerfacecolor='b',linewidth=0.5, markeredgewidth=0.0)
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
    ax4.tick_params(axis='both', direction='in', which='both', right=True, bottom=True, left=True, top=True)


    ax5 = ax4.twinx()
    ax5.plot(xpow_x, plot_dict['XPSPEC-ARG'][0:len(xpow_x)],'ro',markersize=2,linewidth=0.5, markeredgewidth=0.0)
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

    n_seg = int(plot_dict["NSeg"])
    n_seg_plots = int(plot_dict["NPlots"])
    colw = 6

    #grab seg amp
    seg_amp_arr = np.array( plot_dict['SEG_AMP'] )
    seg_ymax = float(plot_dict['Amp'])*3.0 #see generate_graphs.c

    if "ChannelsPlotted" in plot_dict:
        seg_chan_labels = plot_dict["ChannelsPlotted"]
    else:
        #if this info is missing use the default fourfit channel names
        seg_chan_labels =  [chr for chr in "abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ0123456789"]

    # convert SEG_PHS to deg
    seg_phs_arr = np.array( [xx*180/np.pi for xx in plot_dict['SEG_PHS']] )

    # seg_amp_arr1 = seg_amp_arr.reshape(n_seg_plots,n_seg)
    # seg_phs_arr1 = seg_phs_arr.reshape(n_seg_plots,n_seg)

    seg_amp_arr1 = seg_amp_arr.reshape(n_seg, n_seg_plots)
    seg_phs_arr1 = seg_phs_arr.reshape(n_seg, n_seg_plots)

    for ch in range(0,n_seg_plots):
        ax6 = plt.subplot2grid((32,colw*n_seg_plots),(14,colw*ch),rowspan=5,colspan=colw)
        plt.subplots_adjust(wspace=0, hspace=0)
        ax6.plot(range(n_seg), seg_amp_arr1[:,ch],'co-',markersize=2, markerfacecolor='b', linewidth=0.5, markeredgewidth=0.0)
        ax6.set_xlim(0,n_seg)
        ax6.set_ylim(0,seg_ymax)
        ax6.set_xticklabels(labels=[],visible=False)
        ax6.tick_params(axis='both', direction='in', which='both')
        if ch < n_seg_plots-1:
            ax6.title.set_text(seg_chan_labels[ch])
            ax6.title.set_size(9)
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
        ax6a.plot(range(n_seg), seg_phs_arr1[:,ch],'ro',markersize=2, linewidth=0.5, markeredgewidth=0.0)
        ax6a.set_xlim(0,n_seg)
        ax6a.set_ylim(-180,180)
        ax6a.tick_params(axis='both', direction='in', which='both')
        if ch == n_seg_plots-1:
            if n_seg_plots > 1:
                ax6a.title.set_text("All")
                ax6a.title.set_size(9)
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

    #USB/LSB PLOTS
    for ch in range(0,n_seg_plots-1):
        ax7 = plt.subplot2grid((256,colw*n_seg_plots),(152,colw*ch),rowspan=4,colspan=colw)
        plt.subplots_adjust(wspace=0, hspace=0)
        ax7.plot(range(n_seg), np.zeros(n_seg),'co-',markersize=2, markerfacecolor='g', linewidth=0.5, markeredgewidth=0.0)
        ax7.set_xlim(0,n_seg)
        ax7.set_ylim(0,seg_ymax)
        ax7.set_xticklabels(labels=[],visible=False)
        ax7.tick_params(axis='both', direction='in', which='both')
        ax7.xaxis.set_major_locator(plt.NullLocator())
        ax7.yaxis.set_major_locator(plt.NullLocator())
        ax7.set_yticklabels(labels=[],visible=False)
        plt.yticks(visible=False)
        plt.tick_params(left = False, bottom = False)
        if ch == 0:
            ax7.set_ylabel('U',fontsize=7, rotation=0, labelpad=5)
            ax7.yaxis.set_label_coords(-0.23,0.0)

        ax7b = plt.subplot2grid((256,colw*n_seg_plots),(156,colw*ch),rowspan=4,colspan=colw)
        plt.subplots_adjust(wspace=0, hspace=0)
        ax7b.plot(range(n_seg), np.zeros(n_seg),'co-',markersize=2, markerfacecolor='g', linewidth=0.5, markeredgewidth=0.0)
        ax7b.set_xlim(0,n_seg)
        ax7b.set_ylim(0,seg_ymax)
        ax7b.set_xticklabels(labels=[],visible=False)
        ax7b.tick_params(axis='both', direction='in', which='both')
        ax7b.xaxis.set_major_locator(plt.NullLocator())
        ax7b.yaxis.set_major_locator(plt.NullLocator())
        ax7b.set_yticklabels(labels=[],visible=False)
        plt.yticks(visible=False)
        plt.tick_params(left = False, bottom = False)
        if ch == 0:
            ax7b.set_ylabel('L',fontsize=7, rotation=0, labelpad=5)
            ax7b.yaxis.set_label_coords(-0.23,0.0)
            
    #PCAL PLOTS
    for ch in range(0,n_seg_plots-1):
        ax8 = plt.subplot2grid((255,colw*n_seg_plots),(160,colw*ch),rowspan=16,colspan=colw)
        plt.subplots_adjust(wspace=0, hspace=0)
        ax8.plot(range(n_seg), np.zeros(n_seg),'co-',markersize=2, markerfacecolor='m', linewidth=0.5, markeredgewidth=0.0)
        ax8.set_xlim(0,n_seg)
        ax8.set_ylim(-180,180)
        ax8.xaxis.set_major_locator(plt.LinearLocator(numticks=3))
        ax8.xaxis.set_minor_locator(AutoMinorLocator(2))
        ax8.set_xticklabels(labels=[],visible=True)
        ax8.tick_params(axis='both', direction='in', which='both')

        if ch == 0:
            ax8.set_ylabel(r"pcal $\theta$",fontsize=9)
            ax8.yaxis.set_major_locator(plt.FixedLocator([-180, -90, 0, 90, 180]))
            #ax8.yaxis.set_minor_locator(AutoMinorLocator(2))
            ax8.tick_params(axis='y', left = False,  right=False, bottom = False, labelleft=False)
        elif ch == (n_seg_plots-2):
            ax8.yaxis.set_major_locator(plt.FixedLocator([-180, -90, 0, 90, 180]))
            #ax8.yaxis.set_minor_locator(AutoMinorLocator(2))
            ax8.set_yticklabels(ax8.get_yticks(), fontsize=5)
            ax8.tick_params(axis='y', left=False, right=True, labelleft=False, labelright=True)
        else:
            ax8.yaxis.set_major_locator(plt.NullLocator())
            ax8.set_yticklabels(labels=[],visible=False)
            plt.yticks(visible=False)
    
    #TODO FIXME -- make these station labels part of the y-axis title so their placement is done properly no matter the number of channels
    if 'extra' in plot_dict:
        ref_mk4id = plot_dict["extra"]["ref_station_mk4id"]
        rem_mk4id = plot_dict["extra"]["rem_station_mk4id"]
        plt.text(0.925,0.37,ref_mk4id,transform=plt.gcf().transFigure,fontsize=7,verticalalignment='top',family='monospace',horizontalalignment='left',color='g')
        plt.text(0.94,0.37,rem_mk4id,transform=plt.gcf().transFigure,fontsize=7,verticalalignment='top',family='monospace',horizontalalignment='left',color='m')

    #make the text table
    axT = plt.subplot2grid((96,n_seg_plots),(67,0),rowspan=20,colspan=n_seg_plots)
    #plt.subplots_adjust(left=0.07, right=0.93, bottom=0.07, top=0.93)
    #plt.subplots_adjust(left=0.1, right=0.9, top=0.8, bottom=0.1)  # Adjust margins as needed

    ct_header_text = {}
    ct_header_text["#Ch"] = ""
    ct_header_text["Freq(MHz)"] = "Freq (MHz)"
    ct_header_text["Phase"] = "Phase"
    ct_header_text["Ampl"] = "Ampl."
    ct_header_text["SbdBox"] = "Sbd box"
    ct_header_text["APsRf"] = "APs used"
    ct_header_text["APsRm"] = "APs used"
    ct_header_text["PCdlyRf"] = "PC freqs"
    ct_header_text["PCdlyRm"] = "PC freqs"
    ct_header_text["PCPhsRf"] = "PC phase"
    ct_header_text["PCPhsRm"] = "PC phase"
    ct_header_text["PCOffRf"] = "Manl PC"
    ct_header_text["PCOffRm"] = "Manl PC"
    ct_header_text["PCAmpRf"] = "PC amp"
    ct_header_text["PCAmpRm"] = "PC amp"
    ct_header_text["ChIdRf"] = "Chan ids"
    ct_header_text["TrkRf"] = "Tracks"
    ct_header_text["ChIdRm"] = "Chan ids"
    ct_header_text["TrkRm"] = "Tracks"

    #declare the elements which are paired up and printed with a ":"
    paired_data1 = {}
    paired_data2 = {}
    paired_data1["APsRf"] = "APsRm"
    paired_data1["PCPhsRf"] = "PCPhsRm"
    paired_data1["PCOffRf"] = "PCOffRm"
    for k in paired_data1.keys():
        v = paired_data1[k]
        paired_data2[v] = k

    #create the table of per-channel data
    ct_data = []
    ct_headers = plot_dict['PLOT_INFO']['header']
    cth_text = []
    for hdr in ct_headers:
        if hdr in paired_data1.keys():
            hdr2 = paired_data1[hdr]
            hdr_txt = ct_header_text[hdr]
            if hdr_txt != "": #dont plot the empty labels
                #need to grab both data element lists
                elem1 = plot_dict['PLOT_INFO'][hdr]
                elem2 = plot_dict['PLOT_INFO'][hdr2]
                zipped_list = list(zip(elem1, elem2))
                result_list = []
                if hdr == "APsRf":
                    result_list = [f"{int(np.rint(float(item[0])))}/{int(np.rint(float(item[1])))}" for item in zipped_list]
                else:
                    result_list = [f"{int(np.rint(float(item[0])))}:{int(np.rint(float(item[1])))}" for item in zipped_list]
                cth_text.append(hdr_txt)
                ct_data.append(result_list)
        else:
            if not (hdr in paired_data2.keys()):
                hdr_txt = ct_header_text[hdr]
                if hdr_txt != "": #dont plot the empty labels
                    cth_text.append(hdr_txt)
                    if hdr_txt not in ["Chan ids","Tracks"]:
                        result_list = [f"{np.round(float(item), 1)}" for item in plot_dict['PLOT_INFO'][hdr]]
                    else:
                        result_list = [str(item) for item in plot_dict['PLOT_INFO'][hdr]]
                    ct_data.append(result_list)

    #trim the last column (the 'All column') to be added back later
    for ct_row in range(0, len(ct_data)):
        if cth_text[ct_row] in ["Freq (MHz)", "Phase", "Ampl.", "Sbd box"] and len(ct_data[ct_row]) == len(plot_dict['PLOT_INFO']["#Ch"]):
            ct_data[ct_row].pop()

    #add the header names as the last column
    for row in range(0, len(ct_data)):
        hdr_txt = cth_text[row]
        ct_data[row].append(hdr_txt)

    # Create the table
    table = axT.table(cellText=ct_data, loc='center')

    # Remove the borders from the table and set alignment
    for key, cell in table._cells.items():
        cell.set_linewidth(0)
        cell.set_text_props(ha="left")

    # Hide the axis
    axT.axis('off')

    # Adjust the cell font size (optional)
    table.auto_set_font_size(False)
    table.set_fontsize(4)

    # Set the table cell height to make it smaller
    table.scale(1, 0.7)  # Adjust the scale factor as needed




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

    textstr2 = str(plot_dict['Quality'].strip("'")) + '\n\n' + \
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
        plot_dict['Dec'].replace('d', '$\degree$')



    # Add the text boxes
    plt.text(0.83,0.94,textstr1,transform=plt.gcf().transFigure,fontsize=9,verticalalignment='top',
             family='monospace',horizontalalignment='left',color='g')

    plt.text(0.965,0.94,textstr2,transform=plt.gcf().transFigure,fontsize=9,verticalalignment='top',
             family='monospace',horizontalalignment='right',color='k')

    # Add the top matter
    plt.text(0.965,0.98,plot_dict['RootScanBaseline'].strip("'"),transform=plt.gcf().transFigure,
             fontsize=12,verticalalignment='top',family='sans-serif',horizontalalignment='right',fontweight='bold')

    plt.text(0.05,0.98,plot_dict['CorrVers'].strip("'"),transform=plt.gcf().transFigure,
             fontsize=12,verticalalignment='top',family='sans-serif',horizontalalignment='left',fontweight='bold')

    plt.text(0.965,0.96, plot_dict['PolStr'].strip("'"),transform=plt.gcf().transFigure,
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

    bottom_yoffset = 0.16

    # Add the text boxes
    plt.text(0.01,bottom_yoffset,btmtextstr1,transform=plt.gcf().transFigure,fontsize=7,verticalalignment='top',
             family='monospace',horizontalalignment='left',color='k')

    # Add the text boxes
    plt.text(0.28,bottom_yoffset,btmtextstr2,transform=plt.gcf().transFigure,fontsize=7,verticalalignment='top',
             family='monospace',horizontalalignment='right',color='k')

    plt.text(0.3,bottom_yoffset,btmtextstr3,transform=plt.gcf().transFigure,fontsize=7,verticalalignment='top',
             family='monospace',horizontalalignment='left',color='k')

    plt.text(0.6,bottom_yoffset,btmtextstr4,transform=plt.gcf().transFigure,fontsize=7,verticalalignment='top',
             family='monospace',horizontalalignment='right',color='k')

    plt.text(0.65,bottom_yoffset,btmtextstr5,transform=plt.gcf().transFigure,fontsize=7,verticalalignment='top',
             family='monospace',horizontalalignment='left',color='k')

    plt.text(0.88,bottom_yoffset,btmtextstr6,transform=plt.gcf().transFigure,fontsize=7,verticalalignment='top',
             family='monospace',horizontalalignment='right',color='k')

    plt.text(0.89,bottom_yoffset,btmtextstr7,transform=plt.gcf().transFigure,fontsize=7,verticalalignment='top',
             family='monospace',horizontalalignment='left',color='k')

    plt.text(0.97,bottom_yoffset,btmtextstr8,transform=plt.gcf().transFigure,fontsize=7,verticalalignment='top',
             family='monospace',horizontalalignment='right',color='k')

    #this is only true of hops4 generated data (plot_data_dir is missing these items)
    if 'extra' in plot_dict:
        ref_mk4id = plot_dict["extra"]["ref_station_mk4id"]
        ref_az = plot_dict['extra']['ref_station']['az']
        ref_el = plot_dict['extra']['ref_station']['al']
        ref_pa = plot_dict['extra']['ref_station']['pa']
        ref_u = plot_dict['extra']['ref_station']['u']
        ref_v = plot_dict['extra']['ref_station']['v']
        refbtmtextstr = ref_mk4id +":"
        refbtmtextstr += " az " + str(np.round(float(ref_az),1)) 
        refbtmtextstr += " el " + str(np.round(float(ref_el),1))
        refbtmtextstr += " pa " + str(np.round(float(ref_pa),1))
        rem_mk4id = plot_dict["extra"]["rem_station_mk4id"]
        rem_az = plot_dict['extra']['rem_station']['az']
        rem_el = plot_dict['extra']['rem_station']['al']
        rem_pa = plot_dict['extra']['rem_station']['pa']
        rem_u = plot_dict['extra']['rem_station']['u']
        rem_v = plot_dict['extra']['rem_station']['v']
        rembtmtextstr = rem_mk4id +":"
        rembtmtextstr += " az " + str(np.round(float(rem_az),1)) 
        rembtmtextstr += " el " + str(np.round(float(rem_el),1))
        rembtmtextstr += " pa " + str(np.round(float(rem_pa),1))
        du = plot_dict['extra']['u']
        dv = plot_dict['extra']['v']
        uvtextstr = "u,v (fr/asec) " + str(np.round(float(du),3)) + ", " + str(np.round(float(dv),3)) 
        station_coords_textstr = refbtmtextstr + "    " + rembtmtextstr + "    " + uvtextstr
        plt.text(0.01,0.04, station_coords_textstr ,transform=plt.gcf().transFigure,fontsize=7,verticalalignment='top', family='monospace',horizontalalignment='left',color='k')
        plt.text(0.97,0.04, "simultaneous interpolator" ,transform=plt.gcf().transFigure,fontsize=7,verticalalignment='top', family='monospace',horizontalalignment='right',color='k')
        
        control_file = plot_dict['extra']['control_file']
        input_file = plot_dict['extra']['baseline_input_file']
        output_file = plot_dict['extra']['output_file']
        file_info_textstr = "Control file: " + control_file + "   Input file: " + input_file + "   Output file: " + output_file
        plt.text(0.01,0.03, file_info_textstr ,transform=plt.gcf().transFigure,fontsize=6,verticalalignment='top', family='monospace',horizontalalignment='left',color='k')


    #more text at the bottom (RMS/Theor table)
    axT2 = plt.subplot2grid((120,240),(119,5),rowspan=4,colspan=30)
    ct2_rows = 4
    ct2_cols = 2
    ct2_col_label = ['']*ct2_cols
    ct2_col_label[0] = 'RMS'
    ct2_col_label[1] = 'Theor.'
    ct2_row_label = ['-']*ct2_rows
    ct2_row_label[0] = 'ph/seg (deg)'
    ct2_row_label[1] = 'amp/seg (%)'
    ct2_row_label[2] = 'ph/frq (deg)'
    ct2_row_label[3] = 'amp/frq (%)'
    #create the table of per-channel data TODO - FILL ME IN
    ct2_data = [[' - ']*ct2_cols]*ct2_rows
    # Create the table
    table2 = axT2.table(cellText=ct2_data, colLabels=ct2_col_label, rowLabels=ct2_row_label, loc='center')
    # Remove the borders from the table and set alignment
    for key, cell in table2._cells.items():
        cell.set_linewidth(0)
        cell.set_text_props(ha="left")

    # Adjust the cell font size (optional)
    table2.auto_set_font_size(False)
    table2.set_fontsize(7)
    # Hide the axis
    axT2.axis('off')
    # Set the table cell height to make it smaller
    table2.scale(1, 0.7)  # Adjust the scale factor as needed


    #more text at the bottom (amp table)
    axT3 = plt.subplot2grid((120,120),(119,32),rowspan=4,colspan=15)
    ct3_rows = 5
    ct3_cols = 1
    ct3_row_label = ['-']*ct3_rows
    ct3_row_label[0] = 'Amplitude'
    ct3_row_label[1] = 'Search'
    ct3_row_label[2] = 'Interp.'
    ct3_row_label[3] = 'Inc. seg. avg.'
    ct3_row_label[4] = 'Inc. frq. avg.'
    #create the table of per-channel data TODO - FILL ME IN
    ct3_data = [[' - ']*ct3_cols]*ct3_rows
    # Create the table
    table3 = axT3.table(cellText=ct3_data, rowLabels=ct3_row_label, loc='center')

    # Remove the borders from the table and set alignment
    for key, cell in table3._cells.items():
        cell.set_linewidth(0)
        cell.set_text_props(ha="left")
    # Adjust the cell font size (optional)
    table3.auto_set_font_size(False)
    table3.set_fontsize(7)
    axT3.axis('off')
    # Set the table cell height to make it smaller
    table3.scale(1, 0.7)  # Adjust the scale factor as needed


    #more text at the bottom (amp table)
    axT4 = plt.subplot2grid((120,120),(119,119),rowspan=4,colspan=1)
    ct4_rows = 4
    ct4_cols = 2
    ct4_row_label = ['-']*ct4_rows
    ct4_row_label[0] = 'sb window (us)'
    ct4_row_label[1] = 'mb window (us)'
    ct4_row_label[2] = 'dr window (ns/s)'
    ct4_row_label[3] = 'ion window (TEC)'
    #create the table of per-channel data TODO - FILL ME IN
    ct4_data = np.zeros((ct4_rows,ct4_cols)) # [[' - ']*ct4_cols]*ct4_rows

    if 'extra' in plot_dict:
        ct4_data[0][0] = str(np.round(float(plot_dict['extra']['sb_win'][0]),3) )
        ct4_data[0][1] = str(np.round(float(plot_dict['extra']['sb_win'][1]),3) )
        ct4_data[1][0] = str(np.round( float(plot_dict['extra']['mb_win'][0]),3) )
        ct4_data[1][1] = str(np.round( float(plot_dict['extra']['mb_win'][1]),3) )
        ct4_data[2][0] = str( np.round(float(plot_dict['extra']['dr_win'][0]),3) ) 
        ct4_data[2][1] = str( np.round(float(plot_dict['extra']['dr_win'][1]),3) ) 
        ct4_data[3][0] = str( np.round(float(plot_dict['extra']['ion_win'][0]),3) ) 
        ct4_data[3][1] = str( np.round(float(plot_dict['extra']['ion_win'][1]),3) ) 

    print(plot_dict['extra'])

    print( str(plot_dict['extra']['sb_win'][0] ) )
    print( str(plot_dict['extra']['sb_win'][1] ) )


    print(ct4_data)

    # Create the table
    table4 = axT4.table(cellText=ct4_data, rowLabels=ct4_row_label, loc='center')
    # Remove the borders from the table and set alignment
    for key, cell in table4._cells.items():
        cell.set_linewidth(0)
        cell.set_text_props(ha="right")
    # Adjust the cell font size (optional)
    table4.auto_set_font_size(False)
    table4.set_fontsize(7)
    axT4.axis('off')
    # Set the table cell height to make it smaller
    table4.scale(12, 0.7)  # Adjust the scale factor as needed

    #last pile of text
    textstr100 = "Pcal mode: MANUAL, MANUAL   PC period (AP's) X,X" + '\n' + \
        'Pcal rate: X,X (us/s)' + '\n' + \
        'Bits/sample: 2x2 SampCntNorm: disabled' + '\n' + \
        'Data rate(MSamp/s) X MB pts Y Amb Z us ' + '\n' + \
        'Data rate(Mb/s) X    nlags: X t_cohere infinite'


    # Add the text boxes
    plt.text(0.44,0.1,textstr100,transform=plt.gcf().transFigure,fontsize=7,verticalalignment='top',family='monospace',horizontalalignment='left',color='k')


    pylab.show()
    pylab.savefig(filename)
