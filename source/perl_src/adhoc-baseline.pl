#!/usr/bin/perl

use warnings;
use diagnostics;
use strict;
use Getopt::Std;

my %opts;
$opts{'b'} = 'SF';
$opts{'o'} = 'adhoc_'.$opts{'b'};
$opts{'r'} = 0;
$opts{'j'} = 0;
$opts{'v'} = 0;
$opts{'g'} = 60;

my $USAGE="
Usage: $0 [options]

    -a <file>	  to specify an alist file      [NO DEFAULT]
    -o <filebase> to specify output file prefix [$opts{'o'}]
    -b <baseline>                               [$opts{'b'}]

    -r            reverse sense of phase (apply to reference, not remote)

    -j            number of channels            [default: from alist]
    -v            verbose
    -g            max gap in seconds for rms calculation [$opts{'g'}]

The data file should be segmented at an appropriate cadence and put into
time order.  Use 'sort time' in aedit before running this script.

Be VERY careful when using adhoc phase files.  Closure phases and
polarimetric phase differences will be meaningless if consistent adhoc phases
are not applied on all baselines.

";

if ( $#ARGV < 0 || $ARGV[0] eq "--help" || $ARGV[0] eq "-h" ) { print "$USAGE"; exit(0); }
getopts('a:b:g:o:rj:v',\%opts);
my ($alist,$filebase,$targetbaseline,$maxgap);
my ($targetreference,$targetremote,$reverse,$verbose,$nchans);
$alist=$opts{'a'};
$filebase=$opts{'o'};
$targetbaseline=$opts{'b'};
$maxgap=$opts{'g'};
$reverse=$opts{'r'};
$nchans=$opts{'j'};
$verbose=$opts{'v'};
$targetreference=substr($targetbaseline,0,1);
$targetremote=substr($targetbaseline,1,1);

die "Required alist ($alist) is missing\n" if ( ! -f $alist);

my ($i,$j,$nstations,@entry,@station);

$nstations = 2;
$station[0] = $targetreference;
$station[1] = $targetremote;

open (FILIN,$alist);
my (@timecode,@baseline,@ref,@rem,@snr,@phase);
my ($ndata,@doy,@hhmmss,@isfirst,@islast,@tphase);
my (@offset,@rate,@reffreq,@bandcode,@alistchans);
my (@hh,@mm,@ss,@timeindays);
$i = 0;
while (<FILIN>) {
    chomp;
    next if (/^[\*]/);       # skip comments
    @entry    = split();
    next if ($#entry lt 27); # skip partial lines
    $timecode[$i]  = $entry[11];
    $doy[$i]       = substr($timecode[$i],0,3);
    $hhmmss[$i]    = substr($timecode[$i],4,6);
    $hh[$i]        = substr($timecode[$i],4,2);
    $mm[$i]        = substr($timecode[$i],6,2);
    $ss[$i]        = substr($timecode[$i],8,2);
    $timeindays[$i]= ($doy[$i]-1)+($hh[$i]/24)+($mm[$i]/1440)+($ss[$i]/86400);
    $offset[$i]    = $entry[12];
    $baseline[$i]  = $entry[14];
    $bandcode[$i]  = $entry[16];
    $alistchans[$i]= substr($bandcode[$i],1);
    $snr[$i]       = $entry[20];
    $phase[$i]     = $entry[21];
    $rate[$i]      = $entry[27];
    $rate[$i]     *= 1.0e-12;
    $ref[$i]       = substr($entry[14],0,1);
    $rem[$i]       = substr($entry[14],1,1);
    $reffreq[$i]   = $entry[36];
    $reffreq[$i]  *= 1.0e6;
    $tphase[$i]    = $entry[37];
    #die "Segments must be in time order\n" if (($i>0) && ($hhmmss[$i]<$hhmmss[$i-1]) & ($doy[$i]<=$doy[$i-1]));
    if ($i==0){$isfirst[$i] = 1;}
    else {if (($hhmmss[$i]>$hhmmss[$i-1])||($doy[$i]>$doy[$i-1])) {$isfirst[$i] = 1;}
	  else {$isfirst[$i] = 0;}}
    $i++;
}
close(FILIN);
$ndata = $i;

my(@outputfiles,$tempfilename,$thisphase,$thisfile,@filehandles);
my($lastphase,$wrapphase);
for ($i=0;$i<$nstations;$i++){
    $tempfilename = $filebase . "_" . $station[$i];
    local *FILE;
    open FILE,">$tempfilename";
    push (@filehandles, *FILE);
}

my($nwraps,$isfirst);
my($lasttimeindays,$rmsphasediff,$temprmsphasediff,$nrms);
$wrapphase = 0;
$lastphase = 0;
$nwraps = 0;
$isfirst = 1;
$lasttimeindays = $timeindays[0];
$temprmsphasediff = 0;
$nrms = 0;
my($k);
for ($i=0;$i<$ndata;$i++){
    for ($j=0;$j<$nstations;$j++){
	if ($baseline[$i] eq $targetbaseline){
	    if ( ((!$reverse)&&($station[$j] eq $targetreference)) ||
                  (($reverse)&&($station[$j] eq $targetremote)) ){ 
		$thisphase = 0;
	    }
	    if ( ((!$reverse)&&($station[$j] eq $targetremote)) ||
		 (($reverse)&&($station[$j] eq $targetreference)) ){
		if ($station[$j] eq $targetremote){
		    $thisphase = -$phase[$i]+$wrapphase;
		}		  
		else {
		    $thisphase = $phase[$i]+$wrapphase;
		}

		if (!$isfirst){
		    if (abs($thisphase-$lastphase)>180){
			if ($thisphase<$lastphase){
			    $wrapphase += 360;
			    $thisphase += 360;
			    if ($verbose){printf("wrap +\n");}
			}
			else{
			    $wrapphase -= 360;
			    $thisphase -= 360;
			    if ($verbose){printf("wrap -\n");}
			}
			$nwraps++;
		    }
		    if (($timeindays[$i]-$lasttimeindays)<=($maxgap/86400)){
			$temprmsphasediff += ($thisphase-$lastphase)*($thisphase-$lastphase);
			if ($verbose){printf ("%8.2f\n",$temprmsphasediff);}
			$nrms++;
		    }
		}

		die "Segments are not in time order\n" if ($timeindays[$i]<$lasttimeindays);

		$lasttimeindays = $timeindays[$i];
		$lastphase = $thisphase;
		$isfirst = 0;
	    }
            $thisfile = $filehandles[$j];
	    printf $thisfile ("%012.8f",$timeindays[$i]);
            if ($nchans==0){
		for ($k=0;$k<$alistchans[$i];$k++){
		    printf $thisfile (" %6.1f",$thisphase);
		}
	    }
	    else {
		for ($k=0;$k<$nchans;$k++){
		    printf $thisfile (" %6.1f",$thisphase);
		}
	    }
	    printf $thisfile("\n");
	}
    }
}
for ($i=0;$i<$nstations;$i++){
    $thisfile = $filehandles[$i];
    close $thisfile;
}

if ($nwraps>($ndata/6)){
    printf("There were a lot of wraps\n");
}
if ($nrms>0){
    $rmsphasediff = sqrt($temprmsphasediff/$nrms);
    if ($verbose){printf("nrms: %5i\n",$nrms);}
    printf("rms phase difference between consecutive segments: %6.2f deg\n",$rmsphasediff);
}
