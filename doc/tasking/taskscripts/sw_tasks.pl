#!/usr/bin/perl
#
# Main script for a lightweight task management tool.
#
use warnings;           # turns on optional warnings
use diagnostics;        # and makes them not terse
use strict;             # makes unsafe constructs illegal
use Getopt::Std;        # for standard option processing
use File::Path qw(make_path);
#
my %opts;
$opts{'b'} = 'sw_task_bubbles.pl';
$opts{'c'} = 'sw_task_config.pl';
$opts{'d'} = 'png';
$opts{'f'} = 'sw_task_flowchart.pl';
$opts{'g'} = 'ALL';
$opts{'i'} = '-';
$opts{'l'} = 0;
$opts{'o'} = 'tasks';
$opts{'r'} = 'none';
$opts{'s'} = 'uid';
$opts{'t'} = 0;
$opts{'v'} = 0;
$opts{'w'} = 0;
# version 0.0 was APP.
my $VERSION='Task Bubble Machine, version 0.1 for a new HOPS.';
my $USAGE="
Usage: $0 [options]

  where the options are

    -b <file>   bubble color file ($opts{'b'})
    -c <file>   configuration file ($opts{'c'})
    -d <type>   dot graph output type ($opts{'d'})
    -f <file>   flowchart generator file ($opts{'f'})
    -g <dom>    list of domains to graph ($opts{'g'})
    -i <file>   input sw task file ($opts{'i'})
    -l          generate latex ($opts{'l'})
    -o <file>   output sw task root ($opts{'o'})
    -r <what>   what to dump out ($opts{'r'})
    -s <key>    key for output sort ($opts{'s'})
    -t          make plots of things ($opts{'t'})
    -v          verbose (for feedback, $opts{'v'})
    -w          very verbose (for debugging, $opts{'w'})
  
  do 'info dot' for the dot graph types (ps,png,...) and perhaps read
  the dotguide.pdf that is part of the graphviz package everyone uses.

  use -g 'help' for list of possible domains to graph
  choices on -r are: none,all,sum,what,help
  choices on -s are: uid,begin, or one of the task keywords
  the -l flag turns on latex figures and tables in the output directory
  the -f file then provides a script for externally generated figures
  (e.g. gantt charts, &c.)

  a calculated version (*.new) is output when verbose.
  a copy of the input (*.txt) is output when very verbose.
  
  Typical usage:

    ./sw_tasks.pl -i tsk.txt -o fruit/pfx -r sum -g ALL

  makes a directory fruit (if it does not exist) and fills it
  with pfx-<domain>.* files for all domains.  ALL is absolutely
  everything, 'all' is short for all domains.

  Everything is defined in the input file (tsk.txt in this example).
  You can have input be in a set of files (you would manually
  concatenate these--a file directive allows sw_task_split.pl to
  refactor if you need to).

  The -b and -c files exist to allow tuning of default behaviors.
";
# some help and arg parsing
if ( $#ARGV < 0 || $ARGV[0] eq "--help" ) { print "$USAGE"; exit(0); }
if ( $ARGV[0] eq "--version" ) { print "$VERSION" . "\n"; exit(0); }
my @args = @ARGV;
&getopts('b:c:d:g:i:lo:r:s:tvw', \%opts);
my $config = $opts{'c'};
my $dtype  = $opts{'d'};
our $flowcharter = $opts{'f'};
my $graphs = $opts{'g'};
my $input  = $opts{'i'};
my $latex  = $opts{'l'};
our $output = $opts{'o'};
my $report = $opts{'r'};
our $sort_key = $opts{'s'};
my $doth2  = $opts{'t'};
our $bubbles = $opts{'b'};
# verbose for feedback, veryverb for debugging
our $verb = $opts{'v'};
our $veryverb = $opts{'w'};
# force $veryverb to also share things merely $verb.
$verb = 1 if ($veryverb);
# config variables loaded by *config.pl
our ($rundot,$makelegend,$allowautodone,$heuristicshapes);

#
# Main program
#
use lib ".";
require "sw_task_config.pl";
require "sw_task_parser.pl";
require "sw_task_wbs.pl";
require "sw_task_mjd.pl";
require "sw_task_timeline.pl";
require "sw_task_graph.pl";
require "sw_task_latex.pl";
require $config if ( -f $config );

# the work breakdown structure and ultimate tasks
our %wbs;
our %tasks;
our %things;
our %domains;
our ($now_date,$now_mjd,$outputdir);
&set_current_date();

#
# private parsing variables and initial setup
#
my ($hdr,$guts,$ftr,$nkids,@pp);

# provide some help about reports and internals
if ($report eq 'help') {
    &dump_taskage();
    exit 0;
}
if ($sort_key eq 'help') {
    &sort_key_help();
    exit 0;
}

# sanity checks
die "No input file $input\n" if ( ! -s $input );

# allow the output to be to a directory
make_path("$output.dir");
die "Unable to generate output $output\n" if ( ! -d "$output.dir" );
rmdir("$output.dir");
die "Unable to remove $output.dir\n" if ( -d "$output.dir" );
@pp = split(/\//,$output);
$pp[$#pp] = '';
$outputdir = join('/',@pp);
printf("The current date is %s = MJD %d\n", $now_date, $now_mjd) if ($verb);
print "All output appears in '$outputdir'\n";

# scan the input file
($hdr,$guts,$ftr) = &parse_sw_task_file($input);

print "Running DOT on things as well as domains.\n" if ($doth2);

# make an interpreted copy
print "Writing $output.txt\n" if ($veryverb);
&clone_sw_task_file(">$output.txt") if ($verb);
&clone_debugging(">$output.main.dbg") if ($veryverb);

# fill in some blanks and eliminate commentary defaults
print "Filling in the blanks\n" if ($verb);
&canonicalize_items(">$output.canon.dbg");
&make_needs_from_allows();
&make_allows_from_needs();

# work out the timeline
print "Working out the timeline\n" if ($verb);
&work_out_timeline();
&apply_auto_done_rules() if ($allowautodone);
&assign_shape_heuristics() if ($heuristicshapes);

# allows per-thing or per-domain reporting/graphing
# for when we get to the make_domain_graphs() calls
print "Making babies\n" if ($veryverb);
$nkids = &make_kids_of_things(\&by_key);
print "  added $nkids tasks in things\n" if ($veryverb);
$nkids = &make_kids_of_domains(\&by_key);
print "  added $nkids things in domains\n" if ($veryverb);

#
# Dump revised version of input
#
print "Writing $output.new (to compare with inputs)\n" if ($verb);
&dump_new_input(">$output.new","$input") if ($verb);

#
# WBS products -- not clear how much of this is useful
#
if ($report ne 'none') {
    print "Writing reports\n" if ($verb);
    &dump_the_wbs(">$output.wbs", \&by_key, &report_on($report));
    print "  $output.wbs (full details)\n" if ($verb);
    &dump_the_domains(">$output.domain", \&by_key, &report_on($report) );
    print "  $output.domain (report)\n" if ($verb);
    &dump_the_things(">$output.thing", \&by_key, &report_on($report));
    print "  $output.thing (report)\n" if ($verb);
    &dump_the_tasks(">$output.task", \&by_key, &report_on($report));
    print "  $output.task (report)\n" if ($verb);
    &dump_tsv_tasks(">$output.tsv", \&by_uid, &report_on($report));
    print "  $output.tsv (tsv file for export)\n" if ($verb);
}

#
# Make plots of of domains and/or things
#
if ($graphs eq 'ALL') {
    print "Creating absolutely all graphs\n" if ($verb);
    &make_the_graph("$output-ALL",$dtype,1,keys(%tasks));
    &make_domain_graphs($output,$dtype,keys(%domains));
    &make_domain_graphs($output,$dtype,keys(%things)) if ($doth2);
} elsif ($graphs eq 'none') {
    print "No graphs requested\n" if ($verb);
    $makelegend = 0;
} elsif ($graphs eq 'help') {
    printf("Available domains to graph are:\n%10s is everything\n",'ALL');
    for my $d (keys(%domains)) { printf("%10s is %s\n",$wbs{$d}{'nick'},$d); }
    printf("%10s is all of the above, except ALL\n",'all');
    if ($doth2) {
        printf("Available domain%%%%things are:\n");
        for my $d (keys(%things)) {printf("%10s is %s\n",$wbs{$d}{'nick'},$d);}
    }
} else {
    $graphs = join(',',keys(%domains)) if ($graphs eq 'all');
    &make_the_graph("$output-ALL",$dtype,$rundot,keys(%tasks));
    my @d = split(/,/,$graphs);
    print "Creating graphs for " . join(',',@d) . "\n" if ($verb);
    &make_domain_graphs($output,$dtype,@d);
}
# and if requested, a legend
&make_colorkeys($output,$dtype) if ($makelegend);

# Finally generate the latex
&generate_inputs() if ($latex);

print "All done.\n" if ($verb);
exit 0;

#
# debugging support routines
#
sub clone_sw_task_file {
    open(COPY,$_[0]);
    print COPY $hdr;
    print COPY $guts;
    print COPY $ftr;
    close(COPY);
}

sub clone_debugging {
    open(DEBUG,$_[0]);
    print DEBUG "===\n";
    &dump_taskdefs();
    print DEBUG "===\n";
    &dump_abbrevs();
    print DEBUG "===\n";
    close(DEBUG);
}

#
# eof
#
