#!/usr/bin/perl
#
use warnings;           # turns on optional warnings
use diagnostics;        # and makes them not terse
use strict;             # makes unsafe constructs illegal
use Getopt::Std;        # for standard option processing
use File::Path qw(make_path);
#
my %opts;
$opts{'i'} = '-';
$opts{'o'} = './split';
my $VERSION='post-APP';
my $USAGE="
Usage: $0 -i file-to-split.txt -o output-dir

This script may be used to refactor the concatenated single
task file into multiple parts based on the internal file=
directives.  The input file is assumed to be <something>.txt
and will be split into output-dir/<something>_????.txt where
???? starts with 0000 (HEAD) and ends with ZZZZ (TAIL).

Note that there is no consistency checking at present.
";
# some help
if ( $#ARGV < 0 || $ARGV[0] eq "--help" ) { print "$USAGE"; exit(0); }
if ( $ARGV[0] eq "--version" ) { print "$VERSION" . "\n"; exit(0); }
my @args = @ARGV;
&getopts('i:o:', \%opts);
my $input  = $opts{'i'};
my $odir  = $opts{'o'};

my $base = $input;
$base =~ s{(.*).txt}{$1};
my $part = '0000';
my $ofile = $odir . '/' . $base . '_' . $part . ".txt";
my $old = $ofile;

make_path("$odir");
print "input from $input\n";
print "output to $ofile\n";

open(DATA,"<$input");
open(COPY,">$ofile");
while (<DATA>) {
    chomp;
    # start a new output file when the file directive is seen
    # note that there is no checking on the filenames.
    if (/^file\s*=/) {
        $part = $_;
        $part =~ s{file\s*=\s*\S+(....).txt}{$1};
        $ofile = $odir . '/' . $base . '_' . $part . ".txt";
        if ($old =~ m/$ofile/) {
            print "output still to $ofile\n";
        } else {
            close(COPY);
            print "output to $ofile\n";
            open(COPY,">$ofile");
        }
    }
    print COPY $_ . "\n";
}
close(COPY);
close(DATA);

#
# eof
#
