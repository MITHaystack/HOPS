
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

    ax1.plot(dly_x, plot_dict['DLYRATE'],'r-',linewidth=0.8)
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
    #
    # twin both axes, retain the intermediate axes object to adjust those ticks
    ax2a = ax1.twinx()
    plt.yticks(fontsize=8,rotation=90)
    ax2 = ax2a.twiny()
    ax2.plot(mbd_x, plot_dict['MBD_AMP'],'b-',linewidth=0.8)
    ax2.set_xlim(mbd_x[0],mbd_x[-1])
    ax2.set_ylim(bottom=0)
    ax2.set_xlabel(r'multiband delay ($\mu$s)',fontsize=9)
    ax2.xaxis.set_major_formatter(FormatStrFormatter('%.3f'))
    plt.xticks(fontsize=8)
    ax2.xaxis.label.set_color('b')
    ax2.minorticks_on()

    # ax2a.set_ylabel('amplitude',fontsize=9)
    # ax2a.yaxis.label.set_color('b')



    # The singleband delay panel
    ax3 = plt.subplot2grid((16,8),(5,0),rowspan=3,colspan=3)

    ax3.plot(sbd_x, plot_dict['SBD_AMP'],'g-',linewidth=0.8)
    ax3.set_xlim(sbd_x[0],sbd_x[-1])
    ax3.set_ylim(bottom=0)
    ax3.set_xlabel(r'singleband delay ($\mu$s)',fontsize=9)
    plt.xticks(fontsize=8)
    ax3.xaxis.label.set_color('g')
    ax3.xaxis.set_major_formatter(FormatStrFormatter('%.0f'))
    ax3.set_ylabel('amplitude',fontsize=9)
    plt.yticks(fontsize=8,rotation=90)
    ax3.yaxis.label.set_color('g')
    ax3.minorticks_on()


    # The cross-powe sepctra
    ax4 = plt.subplot2grid((16,16),(5,7),rowspan=3,colspan=6)

    ax4.plot(xpow_x, plot_dict['XPSPEC-ABS'],'co-',markersize=3,markerfacecolor='b')
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

    ax5 = ax4.twinx()

    ax5.plot(xpow_x, plot_dict['XPSPEC-ARG'],'ro',markersize=3)
    ax5.set_xlim(xpow_x[0],xpow_x[-1])
    ax5.set_ylim(-180,180)
    ax5.set_ylabel('phase [deg]',fontsize=9)
    ytick_locs = [-180,-90,0,90,180]
    ytick_labels = [str(yy) for yy in ytick_locs]
    plt.yticks(ytick_locs, ytick_labels, fontsize=8,rotation=90)
    ax5.yaxis.label.set_color('r')


    #
    #
    # # Now we build the plots for each band; this will need attention, currently only supports six bands
    # # (and is built by hand; need to loop)
    # cols = 42
    # colw = 6
    #
    # # convert SEG_PHS to deg
    # phs = [xx*180/np.pi for xx in plot_dict['SEG_PHS']]
    #
    # i=0
    # ax6 = plt.subplot2grid((16,cols),(9,0),rowspan=3,colspan=colw)
    # ax6.plot(range(34),plot_dict['SEG_AMP'][i::7],'co-',markersize=3,markerfacecolor='b')
    # ax6.set_xlim(0,34)
    # ax6.set_ylim(0,60)
    # ax6.set_ylabel('amplitude',fontsize=9)
    # plt.xticks(visible=False)
    # plt.yticks(fontsize=8,rotation=90)
    # ax6.yaxis.label.set_color('b')
    # ax6.minorticks_on()
    #
    # ax6a = ax6.twinx()
    # ax6a.plot(range(34), phs[i::7],'ro',markersize=3)
    # ax6a.set_xlim(0,34)
    # ax6a.set_ylim(-180,180)
    #
    #
    # i=1
    # ax7 = plt.subplot2grid((16,cols),(9,colw*i),rowspan=3,colspan=colw)
    # ax7.plot(range(34),plot_dict['SEG_AMP'][i::7],'co-',markersize=3,markerfacecolor='b')
    # ax7.set_xlim(0,34)
    # ax7.set_ylim(0,60)
    # plt.xticks(visible=False)
    #
    # ax7a = ax7.twinx()
    # ax7a.plot(range(34), phs[i::7],'ro',markersize=3)
    # ax7a.set_xlim(0,34)
    # ax7a.set_ylim(-180,180)
    # ax7.set_yticklabels(labels=[],visible=False)
    #
    #
    # i=2
    # ax8 = plt.subplot2grid((16,cols),(9,colw*i),rowspan=3,colspan=colw)
    # ax8.plot(range(34),plot_dict['SEG_AMP'][i::7],'co-',markersize=3,markerfacecolor='b')
    # ax8.set_xlim(0,34)
    # ax8.set_ylim(0,60)
    # plt.xticks(visible=False)
    # ax8.yaxis.label.set_color('b')
    # ax8.minorticks_on()
    #
    # ax8a = ax8.twinx()
    # ax8a.plot(range(34), phs[i::7],'ro',markersize=3)
    # ax8a.set_xlim(0,34)
    # ax8a.set_ylim(-180,180)
    # ax8.set_yticklabels(labels=[],visible=False)
    #
    # i=3
    # ax9 = plt.subplot2grid((16,cols),(9,colw*i),rowspan=3,colspan=colw)
    # ax9.plot(range(34),plot_dict['SEG_AMP'][34*i:34*(i+1)],'co-',markersize=3,markerfacecolor='b')
    # ax9.plot(range(34),plot_dict['SEG_AMP'][i::7],'co-',markersize=3,markerfacecolor='b')
    # ax9.set_xlim(0,34)
    # ax9.set_ylim(0,60)
    # plt.xticks(visible=False)
    # #plt.yticks(visible=False)
    # ax9.yaxis.label.set_color('b')
    # ax9.minorticks_on()
    #
    # ax9a = ax9.twinx()
    # ax9a.plot(range(34), phs[i::7],'ro',markersize=3)
    # ax9a.set_xlim(0,34)
    # ax9a.set_ylim(-180,180)
    # #plt.xticks(visible=False)
    # ax9.set_yticklabels(labels=[],visible=False)
    #
    # i=4
    # ax10 = plt.subplot2grid((16,cols),(9,colw*i),rowspan=3,colspan=colw)
    # #ax10.plot(range(34),plot_dict['SEG_AMP'][34*i:34*(i+1)],'co-',markersize=3,markerfacecolor='b')
    # ax10.plot(range(34),plot_dict['SEG_AMP'][i::7],'co-',markersize=3,markerfacecolor='b')
    # ax10.set_xlim(0,34)
    # ax10.set_ylim(0,60)
    # plt.xticks(visible=False)
    # #plt.yticks(visible=False)
    # ax10.yaxis.label.set_color('b')
    # ax10.minorticks_on()
    #
    # ax10a = ax10.twinx()
    # ax10a.plot(range(34), phs[i::7],'ro',markersize=3)
    # ax10a.set_xlim(0,34)
    # ax10a.set_ylim(-180,180)
    # #ax10a.set_yticklabels(labels=[],visible=False)
    # ax10.set_yticklabels(labels=[],visible=False)
    #
    # i=5
    # ax11 = plt.subplot2grid((16,cols),(9,colw*i),rowspan=3,colspan=colw)
    # #ax11.plot(range(34),plot_dict['SEG_AMP'][34*i:34*(i+1)],'co-',markersize=3,markerfacecolor='b')
    # ax11.plot(range(34),plot_dict['SEG_AMP'][i::7],'co-',markersize=3,markerfacecolor='b')
    # ax11.set_xlim(0,34)
    # ax11.set_ylim(0,60)
    # plt.xticks(visible=False)
    # #plt.yticks(visible=False)
    # ax11.yaxis.label.set_color('b')
    # ax11.minorticks_on()
    #
    # ax11a = ax11.twinx()
    # ax11a.plot(range(34), phs[i::7],'ro',markersize=3)
    # ax11a.set_xlim(0,34)
    # ax11a.set_ylim(-180,180)
    # #plt.yticks(visible=False)
    # ax11.set_yticklabels(labels=[],visible=False)
    #
    # i=6
    # ax12 = plt.subplot2grid((16,cols),(9,colw*i),rowspan=3,colspan=colw)
    # #ax12.plot(range(34),plot_dict['SEG_AMP'][34*i:34*(i+1)],'co-',markersize=3,markerfacecolor='b')
    # ax12.plot(range(34),plot_dict['SEG_AMP'][i::7],'co-',markersize=3,markerfacecolor='b')
    # ax12.set_xlim(0,34)
    # ax12.set_ylim(0,60)
    # plt.xticks(visible=False)
    # #plt.yticks(visible=False)
    # ax12.yaxis.label.set_color('b')
    # ax12.minorticks_on()
    #
    # ax12a = ax12.twinx()
    # ax12a.plot(range(34), phs[i::7],'ro',markersize=3)
    # ax12a.set_xlim(0,34)
    # ax12a.set_ylim(-180,180)
    # ax12a.set_ylabel('phase [deg]',fontsize=9)
    # ytick_locs = [-180,-90,0,90,180]
    # ytick_labels = [str(yy) for yy in ytick_locs]
    # plt.yticks(ytick_locs, ytick_labels, fontsize=8,rotation=90)
    # plt.xticks(visible=False)
    # ax12a.yaxis.label.set_color('r')
    # ax12.set_yticklabels(labels=[],visible=False)
    #
    #
    #
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

    plt.text(0.965,0.92,'pol '+plot_dict['PolStr'].strip("'"),transform=plt.gcf().transFigure,
             fontsize=10,verticalalignment='top',family='sans-serif',horizontalalignment='right',fontweight='bold')


    btmtextstr1 = 'Group delay (usec) ' + '\n' + \
        'Sband delay (usec) ' + '\n' + \
        'Phase delay (usec) ' + '\n' + \
        'Delay rate (us/s) ' + '\n' + \
        'Total Phase (deg) ' + '\n'

    btmtextstr2 = str(np.format_float_scientific( float(plot_dict["GroupDelay"]), precision=11, min_digits=11 ) )  + '\n' + \
        str(np.format_float_scientific(float(plot_dict["SbandDelay(usec)"]), precision=11, min_digits=11) ) + '\n' + \
        str(np.format_float_scientific(float(plot_dict["PhaseDelay(usec)"]), precision=11, min_digits=11) ) + '\n' + \
        str(np.format_float_scientific(float(plot_dict["DelayRate (us/s)"]), precision=11, min_digits=11) ) + '\n' + \
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
        
    btmtextstr8 = str(np.format_float_scientific( float(plot_dict["ResidMbdelayError(usec)"]), precision=2, min_digits=2 ) )  + '\n' + \
        str(np.format_float_scientific(float(plot_dict["ResidSbdelayError(usec)"]), precision=2, min_digits=2) ) + '\n' + \
        str(np.format_float_scientific(float(plot_dict["ResidPhdelayError(usec)"]), precision=2, min_digits=2) ) + '\n' + \
        str(np.format_float_scientific(float(plot_dict["ResidRateError(us/s)"]), precision=2, min_digits=2) ) + '\n' + \
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
