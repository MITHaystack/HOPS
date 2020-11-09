#!/usr/bin/perl
# (c) Massachusetts Institute of Technology, 1994-2020
# The contents of the package Copyright statement apply here.
#
# Code originally authored by Jim Francis for HETE-1.
# (with precious little editing since, by Geoff Crew.)
#
use warnings;           # turns on optional warnings
use diagnostics;        # and makes them not terse
#
my $VERSION='$Id: nightlyerr.pl 3871 2008-12-31 23:53:40Z gbc $';
# script to find & report errors in the nightly build
#
@_=split(/\//,$0); my $ME=pop(@_);
my $USAGE="Usage: $ME nightly-log-file\n";
#
# set defaults for internal variables
#
if ( $#ARGV < 0 || $ARGV[0] eq "--help" ) { print "$USAGE"; exit(0); }
if ( $ARGV[0] eq "--version" ) { print "$VERSION" . "\n"; exit(0); }

#
# Look for barfage, capture context.
#
my $leadin = "";
my $directory = "top-build-level";
my @dirstack = ("");
my $sep = "----------------------------" .
	  "----------------------------";
my $eqc = 0;
my $svn = 1;
my $bwp = 1;
my $phase = "";
my $section = "";
my $count = 0;

# some niceties before we get started
die "No such file '$ARGV[0]'" if ( ! -f $ARGV[0]);
die "Empty file '$ARGV[0]'" if ( ! -s $ARGV[0]);

open NIGHTLY, "$ARGV[0]";

while(<NIGHTLY>)
{
    next if (/^Please report/);

    if (/^make/) {
	if (/^make.*Entering.directory/) {
            $_ =~ s/^make.*Entering/Entering/;
        } elsif (/^make.*Leaving directory/) {
            $_ =~ s/^make.*Leaving/Leaving/;
        }
        # print ">>" . $_;
        if (/^make.*\*\*\*.*/) {
	    next if (/check-recursive/);
	    next if (/check-am/);
	    print $sep . '(' . $section . $phase . ")\n";
	    print "directory = $directory\n";
	    system "grep '^srcdir[ ]=[ ]' $directory/Makefile";
	    print ;
	    print $leadin;
	    $eqc = 0;
            $count ++;
	} elsif (/Entering directory [\`\'](.*)\'/) {
            # print "entering $1\n";
	    push(@dirstack, $directory);
	    push(@dirstack, $1) if ($directory eq "top-build-level");
	    $directory = $1;
	} elsif (/Leaving directory [\`\'](.*)\'/) {
            # print "leaving $1\n";
	    $directory = pop(@dirstack);
	}
	$leadin = "";
    } elsif (s/^Making\s(.*)\sin\s.*\n/$1/) {
	$phase = $_;
    } elsif (s/^%%\s(.*)\n/$1/) {
	$section = $_;
    } elsif (/^===============/) {
	$leadin .= "\t" . $_ if ($eqc == 0);
	$eqc = 1 - $eqc;
    } elsif (/^bwipe:/) {
	print $sep . "(brainwipe:)\n" if ($bwp);
	print ;
	$bwp = 0;
        $count ++;
    } elsif (/^svn-......: [ACM?] /) {
	$section = 'svn-status';
	print $sep . '(' . $section . $phase . ")\n" if ($svn);
	print ;
        $count ++;
	$svn = 0;
    } else {
	$leadin .= "\t" . $_;
    }
}

print $sep . "(finished)\n" if ($count > 0);
close NIGHTLY;

exit(0);

#
# eof
#
