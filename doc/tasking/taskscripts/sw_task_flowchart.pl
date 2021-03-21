#!/usr/bin/perl
#
use warnings;           # turns on optional warnings
use diagnostics;        # and makes them not terse
use strict;             # makes unsafe constructs illegal
use Getopt::Std;        # for standard option processing
use File::Basename;     # for dirname
#
my %opts;
$opts{'p'} = 'dir/pre';
$opts{'n'} = 'nick';
$opts{'d'} = 'dotdev';
$opts{'s'} = 'suffix';
$opts{'e'} = 'false';
$opts{'v'} = 0;
my $VERSION='This is a stub that makes placeholders.';
my $USAGE="
Usage: $0 [options]

  where the options are

    -p <dir/pre>    sw_task product names ($opts{'p'})
    -n <nick>       nickname to process ($opts{'n'})
    -d <dotdev>     (dot) graphic device type ($opts{'d'})
    -s <suffix>     for the plot name ($opts{'s'})
    -e <prog>       some other program to run ($opts{'e'})
    -v              turns on some verbosity ($opts{'v'})

and this program should look at the information in the output
directory (for files of the appropriate prefix) and generate
an output graphic named 'dir/pre-nick-suffix.dotdev'.

";
# some help and arg parsing
if ( $#ARGV < 0 || $ARGV[0] eq "--help" ) { print "$USAGE"; exit(0); }
if ( $ARGV[0] eq "--version" ) { print "$VERSION" . "\n"; exit(0); }
my @args = @ARGV;
&getopts('p:n:d:s:e:v', \%opts);
my $dirpre = $opts{'p'};
my $nick   = $opts{'n'};
my $dotdev = $opts{'d'};
my $suffix = $opts{'s'};
my $myexec = $opts{'e'};
my $verb   = $opts{'v'};

#
# Main program
#
use lib ".";

# inputs is required, of course
die "Need to supply a product directory/prefix\n" if ($dirpre eq 'dir/pre');
die "Need to supply a nickname for domain/thing\n" if ($nick eq 'nick');
die "Unsupported graphs extension\n"
    if ($dotdev ne 'png' and $dotdev ne 'jpg' and $dotdev ne 'pdf');

my $outputdir = dirname($dirpre);
die "No output directory $outputdir\n" if (not -d $outputdir);

# our ($latexgantage,$flowcharter,$flowcharterargs);
#
# if $latexgantage is enabled, our $flowcharter is used to generate gantt
# charts (or whatever you want the figure to be -- infinite complexity here)
# as long as it produces one figure.  The script likely has arguments
# our $flowcharterargs = '-p OUTPUT -n NICK -s flo -d DOTDEV';
# and does the obvious substitutions in this.

# this is a stub program, after all
my $rc = 0;
my $of = "$dirpre-$nick-$suffix.$dotdev";

$rc = not unlink($of) if (-f $of);
die "Unable to overwrite existing $of\n" if ($rc);

print "# survived to execution\n" if ($verb);
if ($myexec eq 'false') {
    print "# Creating the stub $of\n" if ($verb);
    die "No stub plot sw_tasks.png to link\n" if (not -f 'sw_tasks.png');
    $rc = not symlink('../sw_tasks.png', "$of");
} else {
    my @args = ($myexec,'-p',$dirpre,'-n',$nick,'-d',$dotdev);
    print '# Executing:  ' . join(',',@args) . "\n" if ($verb);
    $rc = system(@args);
}
print "# return code $rc\n" if ($verb);
die "Child process to create the graphic failed ($rc)\n" if ($rc != 0);

# this is a program, so it should exit with explicit status
exit($rc);
#
# eof
#
