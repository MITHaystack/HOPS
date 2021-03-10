#!/usr/bin/perl
#
# TTD:
#   consider using tred (optionally) to apply transitive reduction
#   make use of attr for node attributes
#   figure out how to capture edge attributes
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
$opts{'g'} = 'ALL';
$opts{'i'} = '-';
$opts{'o'} = 'tasks';
$opts{'r'} = 'none';
$opts{'s'} = 'uid';
$opts{'t'} = 0;
$opts{'v'} = 0;
$opts{'w'} = 0;
my $VERSION='post-APP';
my $USAGE="
Usage: $0 [options]

  where the options are

    -b <file>   bubble color file ($opts{'b'})
    -c <file>   configuration file ($opts{'c'})
    -d <type>   dot graph output type ($opts{'d'})
    -g <dom>    list of domains to graph ($opts{'g'})
    -i <file>   input sw task file ($opts{'i'})
    -o <file>   output sw task root ($opts{'o'})
    -r <what>   what to dump out ($opts{'r'})
    -s <key>    key for output sort ($opts{'s'})
    -t          make plots of things ($opts{'t'})
    -v          verbose (for feedback, $opts{'v'})
    -w          very verbose (for debugging, $opts{'w'})
  
  do 'info dot' for the dot graph types (ps,png,...)
  use -g 'help' for list of possible domains to graph
  choices on -r are: none,all,sum,what,help
  choices on -s are: uid,begin, or one of the task keywords

  a calculated version (*.new) is output when verbose.
  a copy of the input (*.txt) is output when very verbose.
  
  Typical usage:

    ./sw_tasks.pl -i tsk.txt -o fruit/pfx -r sum -g ALL

  makes a directory fruit (if it does not exist) and fills it
  with pfx-<domain>.* files for all domains.  ALL is absolutely
  everything, 'all' is short for all domains.

  Everything is defined in the input file (tsk.txt in this example)
  except for the -b and -c files.  The uid is a number formed as
  100000 * domainctr + 1000 * thingctr + taskctr
  where domainctr, thinkctr and taskctr are counters assigned as
  the three types are found in the input file.
";
# some help
if ( $#ARGV < 0 || $ARGV[0] eq "--help" ) { print "$USAGE"; exit(0); }
if ( $ARGV[0] eq "--version" ) { print "$VERSION" . "\n"; exit(0); }
my @args = @ARGV;
&getopts('b:c:d:g:i:o:r:s:tvw', \%opts);
my $config = $opts{'c'};
my $dtype  = $opts{'d'};
my $graphs = $opts{'g'};
my $input  = $opts{'i'};
our $output = $opts{'o'};
my $report = $opts{'r'};
our $sort_key = $opts{'s'};
my $doth2  = $opts{'t'};
our $bubbles = $opts{'b'};
# verbose for feedback, veryverb for debugging
our $verb = $opts{'v'};
our $veryverb = $opts{'w'};
our ($rundot,$legend);
$verb = 1 if ($veryverb);

# it is not clear how these must be set
our ($gBegin,$gEnd) = (50000,70000);

#
# Main program
#
use lib ".";
require "sw_task_parser.pl";
require "sw_task_wbs.pl";
require "sw_task_mjd.pl";
require "sw_task_timeline.pl";
require "sw_task_graph.pl";
require $config if ( -f $config );

# the work breakdown structure and ultimate tasks
our %wbs;
our %tasks;
our %things;
our %domains;

# provide some help about reports and internals
if ($report eq 'help') {
    &dump_taskage();
    exit 0;
}

# private parsing variables

# sanity checks
die "No input file $input\n" if ( ! -s $input );

# allow the output to be to a directory
make_path("$output.dir");
die "Unable to generate output $output\n" if ( ! -d "$output.dir" );
rmdir("$output.dir");
die "Unable to remove $output.dir\n" if ( -d "$output.dir" );

# scan the input file
my ($hdr,$guts,$ftr) = &parse_sw_task_file($input);

print "Plotting things too\n" if ($doth2);

# make an interpreted copy
print "Writing $output.txt\n" if ($veryverb);
&clone_sw_task_file(">$output.txt") if ($verb);
&clone_debugging(">$output-main.dbg") if ($veryverb);

# fill in some blanks and eliminate commentary defaults
print "Filling in the blanks\n" if ($verb);
&canonicalize_items();
&make_needs_from_allows();
&make_allows_from_needs();

# work out the timeline
print "Working out the timeline\n" if ($verb);
&work_out_timeline();

# allows per-thing or per-domain reporting/graphing.
print "Making babies\n" if ($verb);
&make_kids_of_things();
&make_kids_of_domains();

# generate new input
print "Writing $output.new\n" if ($verb);
&dump_new_input(">$output.new","$input") if ($verb);

# some real products
if ($report ne 'none') {
    print "Writing reports\n" if ($verb);
    &dump_the_wbs(">$output.wbs", \&by_key, &report_on($report));
    &dump_the_domains(">$output.domain", \&by_key, &report_on($report) );
    &dump_the_things(">$output.thing", \&by_key, &report_on($report));
    &dump_the_tasks(">$output.task", \&by_key, &report_on($report));
    &dump_tsv_tasks(">$output.tsv", \&by_uid, &report_on($report));
}

# make plots of dependencies by domains
if ($graphs eq 'ALL') {
    print "Creating absolutely all graphs\n" if ($verb);
    &make_the_graph("$output-ALL",$dtype,1,keys(%tasks));
    &make_domain_graphs($output,$dtype,keys(%domains));
    &make_domain_graphs($output,$dtype,keys(%things)) if ($doth2);
} elsif ($graphs eq 'none') {
    print "No graphs requested\n" if ($verb);
    $legend = 0;
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
&make_colorkeys($output,$dtype) if ($legend);

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
