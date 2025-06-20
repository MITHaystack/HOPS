fourfit
=======

SYNOPSIS
--------

Performs fringe searching for continuum MkIV data

SYNTAX
------

::

  fourfit [-a] [-b BB:F] [-c controlfile] [-d display device] [-e]
            [-f value] [-m value] [-n value] [-p] [-r afile] [-s naps]
            [-tux] [-P polar_pair] [-T trefoffs] [-X] data file list 
            [set <control file syntax statements>]
         Where all arguments except the data file list are optional.
         The [-r afile] option replaces the data file list, however.
         The "set" argument and the commands which follow it must
         come last.  All option flags must appear before the data file
         list.  Option flags can come in any order.


EXAMPLES
--------

    Here are examples of command-line invocations of fourfit, with
    an explanation of what they do:

::

  fourfit -pt -c control 101-0620/3C279.051V4B

        Test mode, steps through all baselines polarizations for
        this scan.  Without the -pt, the fringes would just be
        written in individual files in 101-0620, with one file
        per baseline-pol-frequencygroup type.

::

  fourfit -txas -m 1 -c control 018-234505 set mb_win -0.0034 .004 freqs a b

        Test mode, xwindow display, accounting switched on, cross
        power spectrum plot switched on, moderately verbose, use
        control file named "control" in current working directory,
        process all data in scan directory 018-234505, override
        multiband delay search window and select channels 'a' and
        'b' only.

::

  fourfit -r refr_list -c control -d hardcopy -b AT:S

        Process all data referenced by type 2 lines in the A-file
        named "refr_list", use control file "control", print the
        fringe plot on the default printer, process only baseline
        AT frequency subgroup S.

OPTION FLAGS
------------

``-a``
    If specified, this option switches on accounting
    of CPU time and wall-clock time used in the various
    parts of fourfit.  When the program finishes, it
    produces a summary of these timing statistics.

``-b BB:F``
    Allows the user to override the control file
    with a specification of the baseline and/or
    frequency group to be processed.  The syntax is
    flexible.  0, 1 or 2 characters before the colon
    refer to the baseline (one character is interpreted
    as a station), and 0 or 1 character after the colon
    is interpreted as the frequency subgroup.  You can
    use the control file wildcard character '?' in
    the baseline, but remember to protect it from the
    C-shell either by escaping it with a backslash '\'
    or enclosing the entire -b argument in single
    quotes.  If you wish only to specify the baseline,
    the colon may be omitted.  An error in the -b
    flag argument causes the flag to be ignored, and
    fourfit will continue execution.

``-c controlfile``
    Specifies the file which contains parameters
    to control the operation of the program.  If
    absent, fourfit will use only the file pointed to
    by the environment variable DEF_CONTROL, which
    in turn defaults to $FF/cf_default as defined
    in the $HOPS/setup.csh file.  Any parameters
    set in a control file specified with the -c option
    override the default file values.  A description
    of the syntax of the control file, with an example,
    can be found later in this document.

``-d display_device``
    Upon completion of a fringe fit, fourfit can
    optionally display the results using postscript.
    The valid choices for "display_device" are:
    ``diskfile:file.ps``  save the plot in "file.ps"
    ``hardcopy``          send the plot directly to lpr
    ``pshardcopy``        print the plot via pplot_print
    ``xwindow``           show the plot in an X11 window
    ``psscreen``          the same, but allow GS_* options

``-e``
    If specified, this option estimates the time required for
    processing by fringing the first scan and extrapolating to
    the remaining scans.  It is equivalent to -a -t with no
    display device, followed by some estimation.  Only the
    first data file mentioned will be included in the estimate.

``-f first channel``
    overrides the default first channel (0) to facilitate
    plotting when there are more than 16 channels (see -n)

``-m value``
    This flag controls the verbosity of the program via
    the integer argument "value", which typically ranges from 3
    (virtually silent except for major errors) to -3 
    (incredibly verbose, of use only to the authors of 
    the program).  The default is 2. An additional mode can
    be toggled on when m=4. This mode prints nothing except
    the name of the generated fringe file, which can be useful
    when calling fourfit from other scripts.

``-n number of channels``
    When channels are overridden (see also the -f flag)
    this tells how many channels to put on the plot. Note
    that neither -f nor -n affect the actual fringe fit,
    just the plotting thereof.

``-p``
    This is equivalent to "-d psscreen".

``-r afile``
    Puts fourfit into "refringe" mode.  Fourfit refringing
    is driven by an A-file input, which overrides any 
    correlator root files directly specified on the command
    line (i.e. the latter are ignored when the -r flag
    is specified).  The input A-file, of which there can
    be only one, may contain root, corel or fringe lines,
    but only the fringe lines are used to determine which
    data to process, by baseline and frequency subgroup.
    Obviously, the -r flag is inconsistent with the -u
    flag, and specifying both is an error.  Note that for
    afiles using HP-1000 (version 1) line formats, fourfit
    has to pre-check the disk for the existence of the 
    type 2 files.  The data area is controlled by the
    DATADIR environment variable.  It defaults to the
    value of $CORDATA.

``-s naps``
    This parameter controls how many AP's are merged
    together into each plotting segment. Thus the number
    of time points shown in the phase, amplitude, and
    validity plots is so controlled. Additionally, the
    ph/seg and amp/seg statistics are calculated based
    upon the stated number of AP's in each segment.

``-t``
    This flag places fourfit in test mode.  Everything
    works as normal, except that the output file is not
    written to disk, and the root file is not updated.
    This is useful when experimenting with different
    fringe-fitting strategies, in order to avoid cluttering
    up the disk.

``-u``
    Normally, fourfit processes all data consistent with
    the data file list and the control information.  When
    this flag is specified, fourfit will also check the
    information in the type-2100 record of the root to 
    see if the data have already been processed by fourfit.
    If so, the data in question are skipped.  The "u"
    stands for update mode.

``-x``
    This is equivalent to "-d xwindow".

``-P pp``
    Controls polarization processing, where the 2 character
    string pp is one of four cross-polarization 
    states: LL, RR, LR, or RL.

``-T trefoffs``
    If this option is invoked, the fourfit reference
    time will be calculated by taking the nominal scan
    start time from the ovex file and adding trefoffs
    (which is an integer # of seconds) to it.

``-X``
    Forces fourfit to write cross-power spectra into
    type 230 records. This option is typically used for
    import into AIPS.

ARGUMENTS
---------

  data file list
            This mandatory argument or arguments tells fourfit
            which data files to process.  The format of the data
            file specification is the standard one for all MkIV
            software.  You may specify individual filenames, 
            scan directories which contain data files, 
            experiment directories, or any combination of
            these three.  In the latter two cases,
            fourfit will descend the directory tree looking for
            data files to add to its internal list of files to
            process.  Only root files need be specified.  The
            data files actually fringe-searched are determined
            by the combination of the root files specified and the
            restrictions imposed by the control file or control
            parameter list (see below).  In the absence of 
            such restrictions, all data associated with the 
            specified root files are processed.

            Beware of trying to specify too many files or scan
            directories, as it is possible to overflow the Unix
            argument list buffer on large experiments.  In such
            cases, specify the experiment directory instead.

        The postscript rendering is performed by ghostscript
        [gs(1)], and GS_* environment variables can be used
        to produce a variety of graphics when the "psscreen"
        (see -d above).  For example,

        GS_OPTIONS=-sOutputFile=abc.png GS_DEVICE=png16 \
        fourfit ...

        will generate a 16-color PNG plot.

        set <control file syntax statements>
            This command line argument is optional, and
            is intended to permit rapid, temporary modification
            of 'fourfit' behaviour without the need to edit the
            control file.  The word "set" tells fourfit to expect
            additional control information on the rest of the
            command line.  The syntax of this control information
            is identical to that of the control file (see
            detailed description below).  The control file
            parser will detect syntax errors and abort if you
            do not follow the rules as laid down.  The control
            information you specify after the "set" argument
            on the command line applies to all data to be
            processed, and overrides whatever the control file
            itself specified for the parameters in question.


ENVIRONMENT
-----------

DEF_CONTROL, DISPLAY, DATADIR, GS_DEVICE, GS_OPTIONS

DESCRIPTION
-----------

Fourfit is the functional analogue of FRNGE on the HP-1000 systems, and
searches the data represented by the root and corel files for fringes,
writing the results of the search to files of type fringe.  The emphasis
in the design of the program has been speed and flexibility, particularly
with regard to future enhancements.  Algorithmically, the program is
closely modelled on FRNGE, with only minor enhancements as yet, based on
the availablity of greater computing resources.

Below is a preliminary form of a document describing in detail the format and
syntax of the control file:

* Example of current syntax for fourfit control file
* This file makes no semantic sense for a real experiment; rather, it is used
* to illustrate typical command syntaxes.


.. code-block:: bash

    ref_freq  8213.15                 * global commands come first
    start -10

    if station L and f_group X
       freqs a+ b c d- e f g h
       pc_phases abcdefgh 5 -11 12 38 -56 13.2 11 -29
       pc_mode ap_by_ap
       pc_freqs abcdefgh 10 10 1010 10 1010 10 1010 1010

    if station L and f_group S
       pc_phases ijkmn 4.5 -78 39 +12 0
       pc_mode normal

    if station A
       pc_mode multitone
       pc_period 30
       pc_tonemask abcdefgh 0 0 8 0 4 0 5 0
       pc_phases_l abcdefgh 12 13 11 12 24 -6 38 110
       pc_phases_r abcdefgh 11 29 14 11 64 -2 44 132
       samplers 2 abcd efgh
       pc_delay_l 30.2
       pc_delay_r -5.9
       ionosphere 18.0

    if (station V or baseline KT) and source 3C279       * parentheses NYI
       sb_win -0.5 0.5    mb_win 0.02 0.02  dr_win -1.0E-6 0.5E-6

    else
       sb_win 0.0 0.0     mb_win 0.02 0.02  dr_win -1.0E-6 0.5E-6

    if scan 288-210210
       sb_win .37 .37

    if scan > 289-132510
       skip true

    if baseline K? and not scan 250-120000 to 251-235959
       switched scan_start
       period 30
       gates abcfgh  0 30  0 10  15 25     0 10  15 25  0 30


    * End of sample control file

SELECTOR KEYWORDS
-----------------

+-------------------+---------------------------------------------------------------+
| KEYWORD           | VALUES                                                        |
+===================+===============================================================+
| station           | 1 character                                                   |
+-------------------+---------------------------------------------------------------+
| baseline          | 2 characters                                                  |
+-------------------+---------------------------------------------------------------+
| source            | string of 1–8 chars                                           |
+-------------------+---------------------------------------------------------------+
| f_group           | 1 character                                                   |
+-------------------+---------------------------------------------------------------+
| scan              | UT-epoch (special format), or:                                |
|                   |                                                               |
|                   | - < UT-epoch                                                  |
|                   | - > UT-epoch                                                  |
|                   | - UT-epoch1 to UT-epoch2 (inclusive time range)               |
+-------------------+---------------------------------------------------------------+

SYNTACTIC KEYWORDS
------------------

+----------------------+
|       KEYWORDS       |
+======================+
| if                   |
+----------------------+
| else (NYI)           |
+----------------------+
| and                  |
+----------------------+
| or                   |
+----------------------+
| not                  |
+----------------------+
| () (NYI)             |
+----------------------+
| <>                   |
+----------------------+
| to                   |
+----------------------+
| ?                    |
+----------------------+


CONTROL ACTION KEYWORDS
-----------------------

+------------------------+--------------------------------------------------------------+
| ACTION KEYWORDS        | VALUES                                                       |
+========================+==============================================================+
| adhoc_amp              | float                                                        |
+------------------------+--------------------------------------------------------------+
| adhoc_file             | string                                                       |
+------------------------+--------------------------------------------------------------+
| adhoc_file_chans       | string                                                       |
+------------------------+--------------------------------------------------------------+
| adhoc_flag_file        | string                                                       |
+------------------------+--------------------------------------------------------------+
| adhoc_period           | float                                                        |
+------------------------+--------------------------------------------------------------+
| adhoc_phase            | 'sinewave', 'polynomial', or 'file'                          |
+------------------------+--------------------------------------------------------------+
| adhoc_poly             | <7 floats/integers (mixture OK)                              |
+------------------------+--------------------------------------------------------------+
| adhoc_tref             | float                                                        |
+------------------------+--------------------------------------------------------------+
| chan_ids               | n char string, followed by n floats                          |
+------------------------+--------------------------------------------------------------+
| dc_block               | 'true' or 'false' (default: false)                           |
+------------------------+--------------------------------------------------------------+
| dec_offset             | float                                                        |
+------------------------+--------------------------------------------------------------+
| delay_offs             | n char string, followed by n floats                          |
+------------------------+--------------------------------------------------------------+
| delay_offs_l           | n char string, followed by n floats                          |
+------------------------+--------------------------------------------------------------+
| delay_offs_r           | n char string, followed by n floats                          |
+------------------------+--------------------------------------------------------------+
| delay_offs_x           | n char string, followed by n floats                          |
+------------------------+--------------------------------------------------------------+
| delay_offs_y           | n char string, followed by n floats                          |
+------------------------+--------------------------------------------------------------+
| dr_win                 | 2 floats                                                     |
+------------------------+--------------------------------------------------------------+
| est_pc_manual          | int                                                          |
+------------------------+--------------------------------------------------------------+
| fmatch_bw_pct          | float                                                        |
+------------------------+--------------------------------------------------------------+
| freqs                  | n chars                                                      |
+------------------------+--------------------------------------------------------------+
| gates                  | n char string, followed by 2n floats                         |
+------------------------+--------------------------------------------------------------+
| gen_cf_record          | 'true' or 'false' (default: false)                           |
+------------------------+--------------------------------------------------------------+
| index                  | n ints                                                       |
+------------------------+--------------------------------------------------------------+
| interpolator           | 'iterate' or 'simul' (default: iterate)                      |
+------------------------+--------------------------------------------------------------+
| ionosphere             | float                                                        |
+------------------------+--------------------------------------------------------------+
| ion_npts               | int                                                          |
+------------------------+--------------------------------------------------------------+
| ion_smooth             | 'true' or 'false' (default: false)                           |
+------------------------+--------------------------------------------------------------+
| ion_win                | 2 floats                                                     |
+------------------------+--------------------------------------------------------------+
| lsb_offset             | float                                                        |
+------------------------+--------------------------------------------------------------+
| mb_win                 | 2 floats                                                     |
+------------------------+--------------------------------------------------------------+
| mbd_anchor             | 'sbd' or 'model' (default: model)                            |
+------------------------+--------------------------------------------------------------+
| min_weight             | float                                                        |
+------------------------+--------------------------------------------------------------+
| notches                | 2n floats                                                    |
+------------------------+--------------------------------------------------------------+
| optimize_closure       | 'true' or 'false' (default: false)                           |
+------------------------+--------------------------------------------------------------+
| passband               | 2 floats                                                     |
+------------------------+--------------------------------------------------------------+
| pc_amp_hcode           | float                                                        |
+------------------------+--------------------------------------------------------------+
| pc_delay_l             | float                                                        |
+------------------------+--------------------------------------------------------------+
| pc_delay_r             | float                                                        |
+------------------------+--------------------------------------------------------------+
| pc_delay_x             | float                                                        |
+------------------------+--------------------------------------------------------------+
| pc_delay_y             | float                                                        |
+------------------------+--------------------------------------------------------------+
| pc_phase_offset_l      | float                                                        |
+------------------------+--------------------------------------------------------------+
| pc_phase_offset_r      | float                                                        |
+------------------------+--------------------------------------------------------------+
| pc_phase_offset_x      | float                                                        |
+------------------------+--------------------------------------------------------------+
| pc_phase_offset_y      | float                                                        |
+------------------------+--------------------------------------------------------------+
| pc_phases              | n char string, followed by n floats                          |
+------------------------+--------------------------------------------------------------+
| pc_phases_l            | n char string, followed by n floats                          |
+------------------------+--------------------------------------------------------------+
| pc_phases_r            | n char string, followed by n floats                          |
+------------------------+--------------------------------------------------------------+
| pc_phases_x            | n char string, followed by n floats                          |
+------------------------+--------------------------------------------------------------+
| pc_phases_y            | n char string, followed by n floats                          |
+------------------------+--------------------------------------------------------------+
| pc_freqs               | n char string, followed by n floats                          |
+------------------------+--------------------------------------------------------------+
| pc_mode                | 'normal', 'ap_by_ap', 'manual', or 'multitone'               |
+------------------------+--------------------------------------------------------------+
| pc_period              | int                                                          |
+------------------------+--------------------------------------------------------------+
| pc_tonemask            | n char string, followed by n floats                          |
+------------------------+--------------------------------------------------------------+
| period                 | int                                                          |
+------------------------+--------------------------------------------------------------+
| plot_data_dir          | string                                                       |
+------------------------+--------------------------------------------------------------+
| ra_offset              | float                                                        |
+------------------------+--------------------------------------------------------------+
| ref_freq               | float                                                        |
+------------------------+--------------------------------------------------------------+
| samplers               | int, followed by up to 8 strings                             |
+------------------------+--------------------------------------------------------------+
| sampler_delay_l        | up to 8 floats                                               |
+------------------------+--------------------------------------------------------------+
| sampler_delay_r        | up to 8 floats                                               |
+------------------------+--------------------------------------------------------------+
| sampler_delay_x        | up to 8 floats                                               |
+------------------------+--------------------------------------------------------------+
| sampler_delay_y        | up to 8 floats                                               |
+------------------------+--------------------------------------------------------------+
| sb_win                 | 2 floats                                                     |
+------------------------+--------------------------------------------------------------+
| skip                   | 'true' or 'false'                                            |
+------------------------+--------------------------------------------------------------+
| start                  | integer                                                      |
+------------------------+--------------------------------------------------------------+
| station_delay          | float                                                        |
+------------------------+--------------------------------------------------------------+
| stop                   | integer                                                      |
+------------------------+--------------------------------------------------------------+
| switched               | 'scan_start' or 'each_minute'                                |
+------------------------+--------------------------------------------------------------+
| t_cohere               | float                                                        |
+------------------------+--------------------------------------------------------------+
| use_samples            | 'true' or 'false'                                            |
+------------------------+--------------------------------------------------------------+
| weak_channel           | float                                                        |
+------------------------+--------------------------------------------------------------+


DEPRECATED
----------

The following keywords are for backward mk4 compatibility only. 

+------------------+-------------------------------------------+
| KEYWORD          | VALUES                                    |
+==================+===========================================+
| max_parity       | float                                     |
+------------------+-------------------------------------------+
| x_crc            | 'keep' or 'discard'                       |
+------------------+-------------------------------------------+
| x_slip_sync      | 'keep', 'discard', or an integer          |
+------------------+-------------------------------------------+
| y_crc            | 'keep' or 'discard'                       |
+------------------+-------------------------------------------+
| y_slip_sync      | 'keep', 'discard', or an integer          |
+------------------+-------------------------------------------+


KEYWORD SEMANTICS
-----------------

.. list-table::
   :widths: 15 85
   :header-rows: 1

   * - KEYWORD
     - VALUES
   * - 
     - **scan selection -- determines if a particular scan/baseline is processed**
   * - skip
     - if this is set to true in the body of an if_block, then any scans matching the if conditions will be skipped.  
       Note: as of 99.2.19 fourfit will not properly skip data if f_group is specified.
   * - 
     - **filtering -- determines whether or not each AP is accepted**
   * - freqs
     - controls which frequency channels get included in the fit.  
       The letters a–p correspond to the order that the frequencies appear in the root file (assuming 16 channels).  
       With no suffix, DSB is implied, if both sidebands are present.  
       A plus suffix denotes USB, a minus is used for LSB.  
       After 26 channels, the uppercase alphabet is used, then 10 digits, finally '$' and '%' (i.e., 64 channels).
   * - start
     - start time for data to be included.
   * - stop
     - stop time for data to be included.  
       Arguments of start and stop are integers with an optional minus sign.  
       A positive integer is interpreted as an absolute time in seconds past the hour (of the scan start time).  
       When a minus sign precedes the start time it is considered to be a time relative to, and later than, the scheduled scan start.  
       Similarly, a negative stop time precedes the scheduled scan stop time, by the indicated number of seconds.
   * - switched
     - turns on (frequency) switched mode, which discards some APs and keeps others, depending on a gating waveform
   * - period
     - period in seconds of the gating waveform
   * - gates
     - for each frequency channel, the starting delay and duration, in seconds, of the gating waveform
   * - passband
     - lower and upper bounds (in MHz) of the spectral passband of data to be accepted, specified as RF frequencies.  
       If the lower bound is greater than the upper bound, the range wraps around—allowing a band in the middle to be excluded.  
       The data is rescaled to preserve the amplitude observable (as if the excluded data were perfectly valid);  
       this means that the area under the cross-power spectral plot amplitude curve is approximately conserved.
   * - notches
     - a list of non-overlapping lower/upper bound pairs (in MHz) to exclude from the spectral passband.  
       (Passband may be applied prior to removal of these notches.)  
       Note that the amplitude modification calculus isn’t sophisticated enough to detect overlaps between passband and notches,  
       so be sure to keep them disjoint. A large number is supported; you'll get a complaint if you exceed it.  
       As with passband, spectral data is rescaled to preserve amplitude observables.
   * - dc_block
     - if set to true, zero out lowest cross-power spectral channel; useful for suppressing DC bias
   * - min_weight
     - fraction of data which must be present for inclusion.  
       Normally, a weight between 0.0 and 1.0 is provided by the correlator to represent the fraction of data actually  
       supporting the correlation value. If you specify a minimum weight, any AP not meeting this threshold will be discarded.
   * - 
     - **search -- control the fringe-searching process**
   * - sb_win
     - single band delay search window bounds, in us
   * - mb_win
     - multiband delay search window bounds, in us; if the upper bound (2nd number) is less 
       than the lower bound (1st number), then fourfit performs a "wrap-around" search, in order to 
       handle the case of a delay near to the multiband (semi-) ambiguity.
   * - dr_win
     - delay_rate search window bounds, in us/s
   * - ion_npts
     - number of evaluation points in ionospheric coarse search
   * - ion_smooth
     - if true, use alternative search on smoothed TEC grid points
   * - ion_win
     - ionospheric coarse search window in TEC units
   * - ra_offset
     - apply right asc. offset (asec) to re-center search windows
   * - dec_offset
     - apply declination offset (asec) to re-center search windows
   * - interpolator
     - selects method of fit interpolation. Classically, an iterative search has been done over sbd, mbd, drate,
       one dimension at a time, for 3 cycles. The simultaneous mode constructs 
       a 5x5x5 cube of data points and does a 3D quintic interpolation.
   * - 
     - **corrections -- apply corrections to the data, either before or after fit**
   * - pc_mode
     - specify phase_cal mode:
       - normal (model linear in time is extracted from the data)
       - manual (specified totally by the user)
       - ap_by_ap (phase cal is extracted independently for each AP) — DEPRECATED: use normal or manual with pc_period 1 or more
       - multitone (all tones in band are coherently fit, and phase is extrapolated to the center of the band)
   * - pc_phases
     - phase_cal phases in deg, for each of the listed freq channels; these offset phases are added to the underlying model, as specified by pc_mode, above. If 2 polarizations are present, the same values are applied to both pols.
   * - pc_phases_l
     - specified in same manner as pc_phases, but the tone phases so specified are applied only to the first pol (L, X, or H)
   * - pc_phases_r
     - specified in same manner as pc_phases, but the tone phases so specified are applied only to the second pol (R, Y, or V)
   * - pc_phases_x
     - synonym for pc_phases_l (see)
   * - pc_phases_y
     - synonym for pc_phases_r (see)
   * - pc_freqs
     - phase cal tone frequencies in KHz, for each of the listed freq channels iff not in range -64..64. 
       Inside of this range, the value is interpreted as a tone #, with 1 being the 1st USB tone, 
       2 being the 2nd USB tone, etc. Negative tone #'s are used for LSB tones.
   * - pc_period
     - in multitone mode (only), the phase can be estimated and applied over each pc_period ap's, 
       thus removing slopes or other drifts in pcal (default is 9999)
   * - pc_tonemask
     - in multitone mode (only): the values for pc_tonemask form a bit-masked map of which tones to 
       exclude for this frequency channel. Thus 1 excludes the lowest tone, 2 the next lower tone, 
       4 the 3rd lowest tone, etc. A value of 5, for example, would exclude the lowest and the 3rd lowest tones (perhaps 10 KHz and 2.01 MHz).
   * - pc_delay_l
     - a time value in ns representing the difference between the travel time from the feed phase 
       center to the pcal injection point, minus the travel time from the pcal pulse generator to 
       the injection point. It is specified separately for the two polarization senses.
   * - pc_delay_r
     - see pc_delay_l
   * - pc_delay_x
     - synonym for pc_delay_l
   * - pc_delay_y
     - synonym for pc_delay_r
   * - pc_phase_offset_x
     - a single additive phase given in degrees, which is applied to the pcal phase 
       of every channel associated with a given polarization
   * - pc_phase_offset_y
     - see pc_phase_offset_x (also pc_phase_offset_l and pc_phase_offset_r)
   * - lsb_offset
     - additive phase in degrees, for the LSB relative to the USB; often necessary when correlating VLBA data against Mk3
   * - ref_freq
     - specifies a frequency in MHz at which the phase delay is determined (default is total LO of first frequency)
   * - adhoc_phase
     - specify mode of ad hoc phase corrections. No corrections are made if this isn't present, or is set to false.
   * - adhoc_period
     - for ad hoc sinewave model; the period in integer seconds
   * - adhoc_amp
     - for ad hoc sinewave model; amplitude in degrees of phase (for ad hoc phase model).
   * - adhoc_tref
     - for both ad hoc phase models; the reference time in seconds past the most recent hour.
   * - adhoc_poly
     - for the ad hoc phase polynomial model; from 1–6 coefficients describing a power-series model in time. (deg/sec^{n})
   * - adhoc_flag_file
     - Name of the file containing adhoc flagging.  Lines of this
       contain times (floating point days from beginning of year) and
       character strings to impose data flagging at a particular time
       (which remains in effect until the next time mentioned).  The
       character string has two characters per channel with a nonzero
       bit for data to be retained with the bit assignments as follows:
       (msb)USB-RL,LSB-RL,USB-LR,LSB-LR,USB-RR,LSB-RR,USB-LL,LSB-LL(lsb)
       If the string is too short, the last byte will be replicated to
       the remaining channels, so a single FF is adquate to retain all
       or a single 00 to discard all.
   * - adhoc_file
     - name of the file containing phases in the ad hoc file mode.
   * - adhoc_file_chans
     - string of channel labels for phases (columns) in the ad hoc file.
   * - use_samples
     - if true, use the sampler statistics (aka state counts) to normalize raw 
       correlation sums to the equivalent analog correlation.
   * - ionosphere
     - specified per station, in TEC (10^{16}/m^{2}) units.
   * - t_cohere
     - coherence time used in fringe fit (default is infinite).
   * - delay_offs
     - delay offsets (ns) to be applied to each listed frequency channel. This correction is made prefit, similar to pcal.
   * - delay_offs_l
     - same as delay_offs, but restricted to LCP.
   * - delay_offs_r
     - same as delay_offs, but restricted to RCP.
   * - delay_offs_x
     - same as delay_offs, but restricted to X linear polarization.
   * - delay_offs_y
     - same as delay_offs, but restricted to Y linear polarization.
   * - samplers
     - number of samplers, followed by freq channel identifiers of channels sharing a sampler, 
       grouped in strings. In multitone mode only, averaged tone-derived differential delays are applied to all grouped channels.
   * - optimize_closure
     - modifies fine fringe search to minimize non-closing delay errors in closure phase;
       may degrade single-baseline fits.
   * - station_delay
     - a priori guess at the delay of the pcal path, from maser
       to the digitizers (ns). Recommended to use 
       sampler_delay_l/r/x/y instead.
   * - mbd_anchor
     - controls how mbd ambiguity is resolved: if 'sbd', uses the one closest to 
       the singleband delay; if 'model', uses one closest to the a priori model.
   * - sampler_delay_l
     - indexed by sampler; the center of the window where the pcal delay ambiguity 
       is resolved. Like 'station_delay' but split by sampler and polarization. For LCP, in ns.
   * - sampler_delay_r
     - same as above, but for RCP instead of LCP.
   * - sampler_delay_x
     - synonym for sampler_delay_l.
   * - sampler_delay_y
     - synonym for sampler_delay_r.
   * - 
     - **miscellaneous functions**
   * - chan_ids
     - changes the assignment of the channel labels
       "abcdef..xyzABC..XYZ01..789$%" from the default values
       to the ones specified by the corresponding list of floats
       This is useful to prevent mis-matched IFs within one experiment
       from causing the same channel label to be ambiguously used on
       various baselines.  (You should ideally use exactly the same
       frequencies as would be assigned by default.)
   * - plot_data_dir
     - if present will be used as a path to dump the plot data
       in a self-documented ascii form to allow export of plot
       information to other arenas (specialized plots, &c).  One
       file per fringe is written; more help is available within
       the files that are produced: there is help for the options
       that may be used to manipulate the content.
   * - fmatch_bw_pct
     - associate frequencies that are within this percentage of bandwidth together (default is 25%).
   * - pc_amp_hcode
     - generate an H code if any phase cal amplitudes are less than this threshold (default is 0.005).
   * - weak_channel
     - the ratio of single_channel_amp to coherent_sum_amp below which a G code is assigned to the scan (default is 0.5).
   * - gen_cf_record
     - if true, saves the full control file in the fringe record.
   * - est_pc_manual
     - if nonzero, estimates manual `pc_phase_?` and `delay_offs_?` values. A positive value estimates the reference station, 
       negative for the remote station. The magnitude is a bitmask:
       - `0x01`: estimate phase  
       - `0x02`: median channel single-band delay  
       - `0x04`: average single-band delay  
       - `0x08`: use total single-band delay  
       - `0x10`: use original per-channel delay  
       - `0x20`: enable heuristics to discard outliers  
       - `0x40`: report phase as `pc_phase_offset_?`  
       - `0x80`: use environment variable `HOPS_EST_PC_BIAS` for phase bias  
       Channels in the `-f` and `-n` range (inclusive) are used. Conflicting options for delay estimation result in no corrections.

SPECIAL KEYWORD VALUES
----------------------

.. list-table::
   :header-rows: 1
   :widths: 20 20

   * - KEYWORD
     - VALUE
   * - ?
     - wild card character
   * - keep
     - 32767
   * - discard
     - 0
   * - true
     - 1
   * - false
     - 0


SPECIAL FORMATS
---------------

   UT-epochs:  
      UT-epochs are expressed in the format ``ddd-hhmmss``, where all 10
      characters are necessary, including leading 0's if
      appropriate.  This format will match that of a scan directory,
      if the UT-epoch that is being specified is an actual scan time.


GENERAL GUIDELINES
------------------

   1) White space is ignored; i.e., multiple spaces and line feeds all
      collapse to a single space.
   2) Multiple commands per line are fine.
   3) Comments: anything from an asterisk through the end of the line
      is ignored.
   4) Nested ifs are not allowed (or necessary). Nested parentheses in
      an if condition are fine (NYI).
      As of 94.1.16, parentheses are not supported. The logical operators,
      in decreasing order of precedence are (not, and, or).
   5) Wildcard "?" matches any single character for f_group, station, or
      baseline, any string (of up to 8 characters) for source, and any
      time-value for scan.
   6) Phase cal and delay offsets are treated station by station. If not
      in a "station context", then values are applied to remote stn only.
   7) Only freqs that are chosen for both stations in a baseline are 
      present in the fit.
   8) If multiple if-blocks match a particular passes' choice of baseline,
      f_group, source, and scan criteria, then the later values assigned
      to each parameter overwrite the earlier ones.
