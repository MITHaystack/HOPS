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
our $rundot;
our $legend;
our $fullnames;
our $skipneedsallows;

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

# TB: top-to-bottom or LR: left-to right ranking
#$graph_rankdir = 'LR';
$graph_rankdir = 'TB';

# PNG resolution is fixed

# orientation dot commands for 'portrait'
$orientation[0] =  "rotate=0\nsize=\"7.5,10\"\n";
$orientation[0] =  "rotate=0\nsize=\"22.5,30\"\n";
# orientation dot commands for 'landscape'
$orientation[1] = "rotate=90\nsize=\"10,7.5\"\n";
$orientation[1] = "rotate=90\nsize=\"30,22.5\"\n";
# orientation of legend ('portrait' or 'landscape')
$orientation[2] = 'portrait';
# orientation of ALL plot
$orientation[3] = 'portrait';

# You can make further adjustments in the *bubbles script
# by adding entries to the %preamble_stuff dictionary.

# controls to run dot or not (sw_tasks.pl).
$rundot = 1;

# controls whether a legend figure is made (sw_tasks.pl)
$legend = 1;

# control node labels (sw_task_graph.pl)
$fullnames = 0;

# skip generated needs/allows (sw_task_graph.pl)
$skipneedsallows = 1;

1;
#
# eof
#
