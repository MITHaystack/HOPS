#!/usr/bin/perl
#
use warnings;           # turns on optional warnings
use diagnostics;        # and makes them not terse
use strict;             # makes unsafe constructs illegal
#
# Allows customization of things that are too detailed for the command line.
#

our $concentrate;
our $edge_needs_clr;
our $edge_allows_clr;
our $graph_rankdir;
our @orientation;
our $same;
our $legend;
our $fullnames;

# set this to one to see the needs and allows arrows separately
# one is sufficient and make it easier to make changes later
if (1) {    # useful for debugging
    $concentrate = 'false';
    $edge_needs_clr = 'red';
    $edge_allows_clr = 'blue';
} else {    # normal case
    $concentrate = 'true';
    $edge_needs_clr = 'black';
    $edge_allows_clr = 'black';
}

# dot graph preamble details sw_task_graph.pl::graph_preamble()

#$graph_rankdir = 'LR';
$graph_rankdir = 'TB';

# orientation dot commands for 'portrait'
$orientation[0] =  "rotate=0\nsize=\"7.5,10\"\n";
# orientation dot commands for 'landscape'
$orientation[1] = "rotate=90\nsize=\"10,7.5\"\n";
# orientation of legend ('portrait' or 'landscape')
$orientation[2] = 'portrait';

# You can make further adjustments in the *bubbles script
# by adding entries to the %preamble_stuff dictionary.

# controls whether peer nodes are fully attributed when sub-plotting
# which doesn't quite work... (sw_tasks.pl)
$same = 1;
# controls whether a legend figure is made (sw_tasks.pl)
$legend = 1;

# control node labels (sw_task_graph.pl)
$fullnames = 1;

1;
#
# eof
#
