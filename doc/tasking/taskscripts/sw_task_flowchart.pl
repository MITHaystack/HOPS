#!/usr/bin/perl
#
use warnings;           # turns on optional warnings
use diagnostics;        # and makes them not terse
use strict;             # makes unsafe constructs illegal
use Getopt::Std;        # for standard option processing
#
my %opts;
# $opts{'?'} = '...';
my $VERSION='This is a stub.';
my $USAGE="
Usage: $0 [options]

  where the options are TBD

";
# some help and arg parsing
if ( $#ARGV < 0 || $ARGV[0] eq "--help" ) { print "$USAGE"; exit(0); }
if ( $ARGV[0] eq "--version" ) { print "$VERSION" . "\n"; exit(0); }
my @args = @ARGV;
&getopts('', \%opts);

#
# Main program
#
use lib ".";

# if $latexgantage is enabled, our $flowcharter is used to generate gantt
# charts or whatever you want the figure to be -- infinite complexity here
# as long as it produces one figure.  The script likely has arguments
# our ($latexgantage,$flowcharter,$flowcharterargs);



# this is a program, so it should exit with explicit status
exit(0);
#
# eof
#
