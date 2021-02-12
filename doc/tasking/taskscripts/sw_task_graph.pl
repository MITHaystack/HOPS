#!/usr/bin/perl
#
use warnings;           # turns on optional warnings
use diagnostics;        # and makes them not terse
use strict;             # makes unsafe constructs illegal
#
# make some graphs of what depends on what
#

require "sw_task_wbs.pl";

our $bubbles;
our %style_preload = ();
require $bubbles if (-f $bubbles);
my $have_preloaded = 0;

our %wbs;
our %domains;
our %things;
our %tasks;
our $sep;
my %ze_attr = ();
my %ze_nodes = ();

our $concentrate;
our $edge_needs_clr;
our $edge_allows_clr;
our $graph_rankdir;
our $fullnames;
#our $preamble_stuff;
our @orientation;

# evens are backgrounds, odds are for fonts
our @clr = (
# 0
    'paleturquoise',    'black',
    'khaki',            'blueviolet',
    'peachpuff',        'blue',
    'palegreen',        'crimson',
    'linen',            'darkslategrey',
# 5
    'magenta',          'black',
    'honeydew',         'darkslateblue',
    'seashell',         'deepskyblue',
    'lavender',         'royalblue',
    'mistyrose',        'forestgreen',
# 10
    'pink',             'black',
    'powderblue',       'darkgoldenrod',
    'lightgoldenrod',   'firebrick',
    'turquoise',        'tomato',
    'coral',            'darkorchid',
# 15
    'dodgerblue',       'paleturquoise',
    'blueviolet',       'khaki',
    'mediumturquoise',  'blue',
#   'crimson',          'palegreen',
    'crimson',          'white',
    'darkslategrey',    'linen',
# 20
    'lightslategrey',   'cornsilk',
    'darkslateblue',    'honeydew',
    'deepskyblue',      'seashell',
    'royalblue',        'lavender',
    'forestgreen',      'mistyrose',
# 25
    'beige',            'tomato',
    'hotpink',          'navyblue',
    'firebrick',        'lightgoldenrod',
    'blanchedalmond',   'darkslateblue',
    'darkorchid',       'coral',
# 30
    'lemonchiffon',     'orangered',
    'papayawhip',       'blueviolet',
    'mintcream',        'orchid',
    'palegoldenrod',    'firebrick',
    'lightsalmon',      'slateblue',
# 35
    'orangered',        'lemonchiffon',
    'darkturquoise',    'violetred',
    'orchid',           'mintcream',
    'mediumslateblue',  'palegoldenrod',
    'slateblue',        'lightsalmon',
# 40
    'springgreen',      'mediumorchid',
    'aliceblue',        'darkslategray',
    'plum',             'darkslategray',
    'azure',            'dimgray',
    'ivory',            'violetred',
# 45
    'mediumorchid',     'springgreen',
    'darkslategray',    'aliceblue',
    'cornflowerblue',   'lavenderblush',
    'dimgray',          'azure',
    'violetred',        'ivory',
# 50
    'tomato',           'black',
#   'lavenderblush',    'cornflowerblue',
#   'darkgoldenrod',    'powderblue',
#   'tomato',           'beige',
#   'cornsilk',         'lightslategrey',
# black, navyblue, cadetblue, sienna, tomato, thistle
# orchid, lightblue, papayawhip, brown, mediumpurple, rosybrown
# darkkhaki, oldlace, blue, darkolivegreen, darkgreen
);

my @node_style = (
    "fillcolor=\"$clr[0]\", fontcolor=\"$clr[1]\" ",
    "fillcolor=\"$clr[2]\", fontcolor=\"$clr[3]\" ",
    "fillcolor=\"$clr[4]\", fontcolor=\"$clr[5]\" ",
    "fillcolor=\"$clr[6]\", fontcolor=\"$clr[7]\" ",
    "fillcolor=\"$clr[8]\", fontcolor=\"$clr[9]\" ",
    "fillcolor=\"$clr[10]\",fontcolor=\"$clr[11]\"",
    "fillcolor=\"$clr[12]\",fontcolor=\"$clr[13]\"",
    "fillcolor=\"$clr[14]\",fontcolor=\"$clr[15]\"",
    "fillcolor=\"$clr[16]\",fontcolor=\"$clr[17]\"",
    "fillcolor=\"$clr[18]\",fontcolor=\"$clr[19]\"",
    "fillcolor=\"$clr[20]\",fontcolor=\"$clr[21]\"",
    "fillcolor=\"$clr[22]\",fontcolor=\"$clr[23]\"",
    "fillcolor=\"$clr[24]\",fontcolor=\"$clr[25]\"",

    "fillcolor=\"$clr[26]\",fontcolor=\"$clr[27]\"",
    "fillcolor=\"$clr[28]\",fontcolor=\"$clr[29]\"",
    "fillcolor=\"$clr[30]\",fontcolor=\"$clr[31]\"",
    "fillcolor=\"$clr[32]\",fontcolor=\"$clr[33]\"",
    "fillcolor=\"$clr[34]\",fontcolor=\"$clr[35]\"",
    "fillcolor=\"$clr[36]\",fontcolor=\"$clr[37]\"",
    "fillcolor=\"$clr[38]\",fontcolor=\"$clr[39]\"",
    "fillcolor=\"$clr[40]\",fontcolor=\"$clr[41]\"",
    "fillcolor=\"$clr[42]\",fontcolor=\"$clr[43]\"",
    "fillcolor=\"$clr[44]\",fontcolor=\"$clr[45]\"",
    "fillcolor=\"$clr[46]\",fontcolor=\"$clr[47]\"",
    "fillcolor=\"$clr[48]\",fontcolor=\"$clr[49]\"",

    "fillcolor=\"$clr[50]\",fontcolor=\"$clr[51]\"",
    "fillcolor=\"$clr[52]\",fontcolor=\"$clr[53]\"",
    "fillcolor=\"$clr[54]\",fontcolor=\"$clr[55]\"",
    "fillcolor=\"$clr[56]\",fontcolor=\"$clr[57]\"",
    "fillcolor=\"$clr[58]\",fontcolor=\"$clr[59]\"",

    "fillcolor=\"$clr[60]\",fontcolor=\"$clr[61]\"",
    "fillcolor=\"$clr[62]\",fontcolor=\"$clr[63]\"",
    "fillcolor=\"$clr[64]\",fontcolor=\"$clr[65]\"",
    "fillcolor=\"$clr[66]\",fontcolor=\"$clr[67]\"",
    "fillcolor=\"$clr[68]\",fontcolor=\"$clr[69]\"",

    "fillcolor=\"$clr[70]\",fontcolor=\"$clr[71]\"",
    "fillcolor=\"$clr[72]\",fontcolor=\"$clr[73]\"",
    "fillcolor=\"$clr[74]\",fontcolor=\"$clr[75]\"",
    "fillcolor=\"$clr[76]\",fontcolor=\"$clr[77]\"",
    "fillcolor=\"$clr[78]\",fontcolor=\"$clr[79]\"",

    "fillcolor=\"$clr[80]\",fontcolor=\"$clr[81]\"",
    "fillcolor=\"$clr[82]\",fontcolor=\"$clr[83]\"",
    "fillcolor=\"$clr[84]\",fontcolor=\"$clr[85]\"",
    "fillcolor=\"$clr[86]\",fontcolor=\"$clr[87]\"",
    "fillcolor=\"$clr[88]\",fontcolor=\"$clr[89]\"",

    "fillcolor=\"$clr[90]\",fontcolor=\"$clr[91]\"",
    "fillcolor=\"$clr[92]\",fontcolor=\"$clr[93]\"",
    "fillcolor=\"$clr[94]\",fontcolor=\"$clr[95]\"",
    "fillcolor=\"$clr[96]\",fontcolor=\"$clr[97]\"",
    "fillcolor=\"$clr[98]\",fontcolor=\"$clr[99]\"",

    "fillcolor=\"$clr[100]\",fontcolor=\"$clr[101]\"",
);
my %style_reserved = ();
my $style_counter = 0;
my $styles_used = 0;
my $ze_defs = '';

#
# Preload certain attributes.  This way we can force repeatability
# for some of the colors.  Note that indexing is by task name, so
# need to get the spelling correct...
#
sub preload_attr {
    my ($p);
    return if ($have_preloaded);
    $ze_defs .= "// preload\n";
    for my $p (keys(%style_preload)) {
        $ze_attr{$p} = $style_preload{$p};
        $ze_defs .= "// '$p' => " . $ze_attr{$p} . ", \n";
        $style_reserved{$ze_attr{$p}} = $p;
    }
    $ze_defs .= "// auto\n";
    $have_preloaded = 1;
}

#
# makes a name a legal identifier for dot -- turn hyphens into
# underscores, and delete any other offending characters.
#
sub make_it_legal {
    my $item = $_ = $_[0];
    s/^\s+//;
    s/\s+$//;
    s/\s/_/g;
    tr{-: ./()<>}{__}d;
    if ($_ eq '') {
        print "item '$item' nuked\n" if ($_ eq '');
        return 'Some_Node';
    }
    return $_;
}

#
# set up general graph attributes
#
sub graph_preamble {
    my ($pl,$k) = @_;
    my $stuff = '';
    if (!defined($pl)) { $pl = 1; }
    elsif ($pl eq 'portrait') { $pl = 0; }
    elsif ($pl eq 'landscape') { $pl = 1; }
    else { $pl = 0; }
    $stuff .= "//\n";
    $stuff .= "// graph_preamble\n";
    $stuff .= "//\n";
    $stuff .= "rankdir=$graph_rankdir;\n";
    $stuff .= "concentrate=$concentrate\n";
#   $stuff .= $preamble_stuff;
    $stuff .= $orientation[$pl];
    $stuff .= "node [style=filled,color=black,fillcolor=lightgray];\n";
    $stuff .= "\n";
    return $stuff;
}
#
# final stuff
#
sub graph_postscript {
    my $stuff = '';
    $stuff .= "\n";
    $stuff .= "//\n";
    $stuff .= "// graph_postscript\n";
    $stuff .= "//\n";
    $stuff .= "// $styles_used\n";
    $stuff .= $ze_defs;
    $stuff .= "//\n";
    return $stuff;
}

#
# actually run dot and make the plot
#
sub use_dot_file {
    my ($name,$type,$cmd,$st) = @_;
    $cmd = "dot -T$type $name.dot -o $name.$type";
    $st = system($cmd);
    return($st);
}

#
# find the next available style counter, wrapping around to the
# beginning when we run out
#
sub next_style_counter {
    my $counter;
    $styles_used++;
    while (defined($style_reserved{$style_counter})) {
        $style_counter++;
        $style_counter = 0 if ($style_counter == ($#node_style + 1));
    }
    $counter = $style_counter;
    $style_counter++;
    $style_counter = 0 if ($style_counter == ($#node_style + 1));
    return($counter);
}

#
# provides attributes for a node based on heritage
#
sub node_attr {
    my ($n,$k,$p,$t,$s) = @_;   # node, key, parent, dot-text, label-string
    &preload_attr();
    if (!defined($ze_attr{$p})) {
        $ze_attr{$p} = &next_style_counter();
        $ze_defs .= "// '$p' => " . $ze_attr{$p} . ", \n";
    }
    $s = $wbs{$k}{'start'} . '\n' . $wbs{$k}{'stop'};
    $s .= '\n' . $wbs{$k}{'done'} . '%';
    if ($fullnames) {
        my @pt = split(/$sep/,$k);
        $t = 'label="' . $pt[2] . '\n' . $s . '", ';
    } else {
        $t = 'label="\N\n' . $s . '", ';
    }
    $t .= $node_style[$ze_attr{$p}];
    return($t);
}

#
# generates text for a new node.  we get the node attributes from the parent.
# %ze_nodes is a cache of nodes as defined so that we save ourselves alot
# of construction work and it allows us to color out-of-domain nodes correctly.
#
sub new_node {
    my ($n,$k,$p) = @_;     # node, key, parent
    if (defined($ze_nodes{$n})) { return($ze_nodes{$n}); }
    $p = $wbs{$k}{'parent'};
    my $t = "$n [" . &node_attr($n,$k,$p) . "]\n";
    $ze_nodes{$n} = $t;
    return($t);
}

#
# Called to make a plot of dependencies
#
sub make_the_graph {
    my ($name,$type,$plot,@nodes) = @_;
    my ($key,$node,$nick,$item,$doma,$peer,@list,%peerage,%locals,@ddid);
    my ($orio);
    return(1) if ($#nodes < 0);

    @list = split(/-/,$name);
    $doma = &task_by_nick($nick = $list[1]);
    $orio = $wbs{$doma}{'orient'};
    $doma =~ s/$sep/\\n/;

    open DOT,">$name.dot";
    print DOT "digraph $nick {\n";
    print DOT "//\n";
    print DOT "// $name.dot\n";
    print DOT "//\n";
    print DOT &graph_preamble($orio,$name);

    print DOT '"' . $doma . '"' . " [fontsize=24,shape=box];\n";

    for $key (@nodes) {
        push(@ddid,$key);
        $nick = $wbs{$key}{'nick'};
        $node = &make_it_legal($nick);
        print DOT "// $key => $nick => $node\n";
        print DOT &new_node($node, $key);
        $locals{$node} = 1;

        @list = split(/,/,$wbs{$key}{'needs'});
        print DOT ' edge[color="' . $edge_needs_clr .
            '"] // needs' .";\n" if ($#list >= 0);
        for $item (@list) {
            $peer = &make_it_legal($item);
            print DOT ' ' . $peer .' -> '. $node .";\n";
            $peerage{$peer} = 1;
            push(@ddid,&task_by_nick($item));
        }

        @list = split(/,/,$wbs{$key}{'allows'});
        print DOT ' edge[color="' . $edge_allows_clr .
            '"] // allows' .";\n" if ($#list >= 0);
        for $item (@list) {
            $peer = &make_it_legal($item);
            print DOT ' ' . $node .' -> '. $peer .";\n";
            $peerage{$peer} = 1;
            push(@ddid,&task_by_nick($item));
        }

    }
    print DOT "// nodes from other domains\n";
    for $peer (keys(%peerage)) {
        if (defined($ze_nodes{$peer}) and not defined($locals{$peer})) {
            print DOT $ze_nodes{$peer};
        }
    }

    print DOT &graph_postscript();
    print DOT "}\n";
    close DOT;

    # make a legend file and per-domain task details
    &make_legend($name,$doma,@ddid);

    return(&use_dot_file($name,$type)) if ($plot);  # run dot
    return(0);
}

sub tasks_of_domain {
    my $domain = $_[0];
    my @dtsks = ();
    for my $thing (keys(%things)) {
        if ($wbs{$thing}{'parent'} eq $domain) {
            my @more = split(/,/,$wbs{$thing}{'kids'});
            for my $m (@more) {
                next if ($m eq '');
                push(@dtsks,&task_by_nick($m));
            }
        }
    }
    return (@dtsks);
}

sub tasks_of_thing {
    my $thing = $_[0];
    my @dtsks = ();
    for my $tsk (keys(%tasks)) {
        if ($wbs{$tsk}{'parent'} eq $thing) {
            push(@dtsks,$tsk);
        }
    }
    return (@dtsks);
}

#
# loop over the domains
#
sub make_domain_graphs {
    my ($output,$dtype,@dlist) = @_;
    my ($domnick,$domain,$tsk,@dtsks);
    for $domain (@dlist) {

        if (defined($wbs{$domain})) {
            $domnick = $wbs{$domain}{'nick'};
        } else {
            $domnick = $domain;
            $domain = &task_by_nick($domain);
            if ($domain eq 'none') {
                print "'$domnick' -> 'domain' in make_domain_graphs\n";
                next;
            }
        }

        if ($wbs{$domain}{'type'} eq 'domain') {
            @dtsks = &tasks_of_domain($domain);
        } elsif ($wbs{$domain}{'type'} eq 'thing') {
            @dtsks = &tasks_of_thing($domain);
        }

        if ($#dtsks < 0) {
            print "Nothing to graph for $domnick\n";
            next;
        }
        print "Problem with DOT-$domnick ($#dtsks)\n" if (
            &make_the_graph("$output-$domnick",$dtype,1,@dtsks));
    }
}

#
# Make a thing-color legend.
#
sub make_colorkeys {
    my ($name,$type) = @_;
    open DOT,">$name-legend.dot";
    print DOT "graph legend  {\n";
    print DOT &graph_preamble($orientation[2],'legend');

    for my $n (sort(keys(%style_preload))) {
        $_ = $n;
        s/%%/\\n/;
        print DOT "color_" . sprintf("%02d", $style_preload{$n}) .
            " [" .  'label="\N\n' . $_ . '", ' .
            $node_style[$style_preload{$n}] . "]\n";
    }
    for my $n (0..4) {
        my $k = $n + 1;
        print DOT "color_" . $n . '0' . ' -- ' .
                  "color_" . $k . '0' . "\n" if ($k < 5);
        for my $m (0..8) {
            my $p = $m + 1;
            print DOT "color_" . $n . $m . ' -- ' .
                      "color_" . $n . $p . "\n";
        }
    }
    print DOT "{ rank=same; ";
    for my $n (0..4) { print DOT "color_" . $n . '0; '; }
    print DOT "}\n";

    print DOT &graph_postscript();
    print DOT "}\n";
    close DOT;
    return(&use_dot_file("$name-legend",$type));  # run dot
}

#
# generate snippet of new task file while making the legend
#
sub make_legend_keys {
    my $name = shift(@_);
    my $doma = shift(@_);
    open NEW, ">$name.task";
    print NEW "HEAD of tasks for: $doma\n\n";
    for my $task (sort { $wbs{$a}{'line'} <=> $wbs{$b}{'line'} } @_) {
        &write_new_rest($task);
    }
    print NEW "\nTAIL of tasks for: $doma\n";
    close NEW;
}

#
# make a key file since the bubbles are cryptic
#
our $desc;
format KEY =
      (what) ^<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<
             $desc
~~           ^<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<
             $desc
.
#
sub make_legend {
    my $name = shift(@_);
    my $doma = shift(@_);
    my @tlist = ();
    open KEY, ">$name.key";
    my %done;
    for my $task (@_) {
        next if (defined($done{$task}));
        push @tlist,$task if ($wbs{$task}{'domain'} eq $doma);
        $done{$task} = 1;
        print KEY $wbs{$task}{'nick'} . " refers to:\n";
        print KEY '  (domain) ' . $wbs{$task}{'domain'} . "\n";
        print KEY '    (thing) ' . $wbs{$task}{'thing'} . "\n";
        print KEY '      (task) ' . $wbs{$task}{'task'} . "\n";
        print KEY '      (when) ' .
            $wbs{$task}{'start'} . " - " . $wbs{$task}{'stop'} . ' is ' .
            $wbs{$task}{'done'} . "% complete\n";
        $desc = $wbs{$task}{'desc'};
        write KEY;
        print KEY "\n";
    }
    close KEY;
    &make_legend_keys($name,$doma,@tlist);
}

#
# this is a library module, so it needs to return TRUE.
#
1;
#
# eof
#
