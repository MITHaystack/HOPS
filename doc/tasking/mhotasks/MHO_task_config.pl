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
#our $preamble_stuff;
our @orientation;
our $same;
our $legend;
our $fullnames;

if (0) {    # useful for debugging
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

# if (0) { # landscape for ps
#     $preamble_stuff = "rotate=0\nsize=\"7.5,10\"\n";
# } elsif (0) { # used alot for ps
#     $preamble_stuff = "rotate=90\nsize=\"10,7.5\"\n";
# } elsif (0) { # landscape, no size
#     $preamble_stuff = "rotate=0\nsize=\"10,16\"\n";
# } elsif (0) {
#     $preamble_stuff = "rotate=90\nsize=\"11,8.5\"\n";
# } elsif (0) { # 1st guess for poster printer chopped.
#     $preamble_stuff = "rotate=90\nsize=\"34,34\"\n";
# } elsif (0) { # 2nd guess for poster printer chopped.
#     $preamble_stuff = "rotate=0\nsize=\"34,10\"\n";
# } elsif (0) { # landscape, no size, normal for screen viewing
#     $preamble_stuff = "rotate=0\n";
# } else { # normal for png
#     $preamble_stuff = "rotate=90\n";
# }

# controls whether peer nodes are fully attributed when sub-plotting
# which doesn't quite work...
$same = 1;
$legend = 1;
$fullnames = 0;

1;
#
# eof
#
