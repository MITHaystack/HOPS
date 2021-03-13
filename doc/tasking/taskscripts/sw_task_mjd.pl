#!/usr/bin/perl
#
use warnings;           # turns on optional warnings
use diagnostics;        # and makes them not terse
use strict;             # makes unsafe constructs illegal
#
# Support script for computing timeline dates
#
# $ HOPS_JULIAN_EPOCH=UTC hops_time -q I 56763.0 |cut -dT -f1
# $ HOPS_JULIAN_EPOCH=UTC hops_time -q M 2014-04-16 |cut -d. -f1|sed 's/$/.0/'
#
# Calendar dates (from APP) are of the form YYYY-MM-DD <=> MJD (5 digits)
# 2011-08-15 when contract awarded  55788
# 2012-02-29 (leap day)             55986
# 2016-02-29 (leap day)             57447
# 2016-08-15 six years afterwards   57615
#

our $date_pat = '[0-9]{4}-[0-9]{2}-[0-9]{2}';
our $mjd_pat  = '[0-9]{5}';

my $testing = defined($ENV{"TEST_MJD"});

#
# support routines--many ways to do this--but this is adequate.
#
my %mjdstart = (
    '2010' => 55197, '2011' => 55562, '2012' => 55927, '2013' => 56293,
    '2014' => 56658, '2015' => 57023, '2016' => 57388, '2017' => 57754,
    '2018' => 58119, '2019' => 58484, '2020' => 58849, '2021' => 59215,
    '2022' => 59580, '2023' => 59945, '2024' => 60310, '2025' => 60676,
    '2026' => 61041, '2027' => 61406, '2028' => 61771, '2029' => 62137,
    '2030' => 62502, '2031' => 62867, '2032' => 63232, '2033' => 63598,
    '2034' => 63963, '2035' => 64328, '2036' => 64693, '2037' => 65059,
    '2038' => 65424, '2039' => 65789, '2040' => 66154, '2041' => 66520,
    '2042' => 66885, '2043' => 67250, '2044' => 67615, '2045' => 67981,
    '2046' => 68346, '2047' => 68711, '2048' => 69076, '2049' => 69442,
    '2050' => 69807, '2051' => 70172, '2052' => 70537, '2053' => 70903,
);
my %nleap = (
    '01' =>   1, '02' =>  32, '03' =>  60,
    '04' =>  91, '05' => 121, '06' => 152,
    '07' => 182, '08' => 213, '09' => 244,
    '10' => 274, '11' => 305, '12' => 335,
);
my %yleap = (
    '01' =>   1, '02' =>  32, '03' =>  61,
    '04' =>  92, '05' => 122, '06' => 153,
    '07' => 183, '08' => 214, '09' => 245,
    '10' => 275, '11' => 306, '12' => 336,
);
sub date_to_mjd {
    my ($date,$yy,$mm,$dd,$mjd) = $_[0];
    $yy = $date;    $yy =~ s/^(....)-..-../$1/;
    $mm = $date;    $mm =~ s/^....-(..)-../$1/;
    $dd = $date;    $dd =~ s/^....-..-(..)/$1/;
    $mjd = int($mjdstart{$yy});
    if ($yy % 4) { $mjd += $nleap{$mm}; } else { $mjd += $yleap{$mm}; }
    $mjd += $dd - 2;
    return($mjd);
}
sub mjd_to_date {
    my ($mjd,$yy,$mm,$dd) = ($_[0],2010,0,1);
    while ($yy < 2053) {
        if ($mjd >= $mjdstart{$yy} and $mjd < $mjdstart{$yy+1}) { last; }
        $yy++;
    }
    $mjd -= ($mjdstart{$yy} - 2);
    if ($yy % 4) {
        while ($mm < 12 and $mjd > $nleap{sprintf("%02d",1+$mm)}) { $mm++; }
        $mjd -= $nleap{sprintf("%02d",$mm)};
    } else {
        while ($mm < 12 and $mjd > $yleap{sprintf("%02d",1+$mm)}) { $mm++; }
        $mjd -= $yleap{sprintf("%02d",$mm)};
    }
    $dd = $mjd;
    return(sprintf("%04d-%02d-%02d", $yy,$mm,$dd));
}

# for debugging:
# (echo ...) | TEST_MJD=true perl -- sw_task_mjd.pl
if ($testing) {
  while (<>) {
    chomp;
    if (m/$date_pat/) {
        my $mjd = &date_to_mjd($_);
        my $date = &mjd_to_date($mjd);
        print $_ ." -> ". $mjd . " -> " . $date;
        if ($date eq $_) { print " (ok)\n"; } else { print " (!!)\n"; }
    } elsif (m/$mjd_pat/) {
        my $date = &mjd_to_date($_);
        my $mjd = &date_to_mjd($date);
        print $_ ." -> ". $date . " -> " . $mjd;
        if ($mjd eq $_) { print " (ok)\n"; } else { print " (!!)\n"; }
    } else {
        print $_ . " does not match\n";
    }
  }
}

#
# this is a library module, so it needs to return TRUE.
#
1;
#
# eof
#
