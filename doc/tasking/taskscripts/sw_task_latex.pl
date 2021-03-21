#!/usr/bin/perl
#
use warnings;           # turns on optional warnings
use diagnostics;        # and makes them not terse
use strict;             # makes unsafe constructs illegal

# common controls
our ($verb,$veryverb,$doth2,%wbs);
# $outputdir is the directory; $output is the directory/prefix
our ($outputdir,$output);

# latex support is conditional on these:
our $latexfigures;      # generate the figures
our $latextables;       # generate the tables
our $latexflowcharts;   # generate flowcharts (Gantt)
our $latexinputs;       # generate the input files
our $latexfile;         # name of the main file

# $latexflowcharts requires these to do the work
our ($flowcharter,$flowcharterargs);

# working variables; every domain or thing gets a file in uid sort order
my @latexfiles = ();

sub latex_flows {
    my ($key,$nick,$legal,$type) = @_;
    my ($rc,$arg,@args);
    my $full = $wbs{$key}{'domain'};
    $full .= ' : ' . $wbs{$key}{'thing'};
    $full .= ' : ' . $wbs{$key}{'task'};
    return('') if ($latexflowcharts == 0);
    open(FLO, ">$outputdir/$legal-flo.tex");
    push(@latexfiles, "$legal-flo.tex");
    $arg = $flowcharterargs;
    $arg =~ s/OUTPUT/$output/;
    $arg =~ s/NICK/$nick/;
    $arg =~ s/DOTDEV/png/;
    @args = split(/ /,$arg);
    unshift(@args, "./$flowcharter");
    print FLO "% figure for flow (Gantt) chart for $nick\n";
    print FLO "% $full\n";
    $rc = system(@args);
    print FLO "% via system(" . join(',',@args) . ") = $rc.\n";
    if (-f "$output-$nick-flo.png") {
        print FLO "\\FIXME[includegraphics...]\n";
    } else {
        print FLO "\\FIXME[flow (Gantt) chart for $nick failed]\n";
    }
    print FLO "% eof\n";
    close(FLO);
    return("Some words about the flow (Gantt) chart for $nick\n");
}

# include the generated figure graphic in a figure float with caption
sub latex_figure {
    my ($key,$nick,$legal,$type) = @_;
    my $full = $wbs{$key}{'domain'};
    $full .= ' : ' . $wbs{$key}{'thing'};
    $full .= ' : ' . $wbs{$key}{'task'};
    return('') if ($latexfigures == 0);
    open(FIG, ">$outputdir/$legal-fig.tex");
    push(@latexfiles, "$legal-fig.tex");
    print FIG "% figure for $nick\n";
    print FIG "% $full\n";
    print FIG "\\FIXME[figure for $nick would go here]\n";
    print FIG "% eof\n";
    close(FIG);
    return("intro to figure for $nick\n");
}

# tasks get task tabular details
sub latex_table_task {
    my ($key,$nick,$legal,$type) = @_;
    my $full = $wbs{$key}{'domain'};
    $full .= ' : ' . $wbs{$key}{'thing'};
    $full .= ' : ' . $wbs{$key}{'task'};
    open(TAB, ">$outputdir/$legal-tab.tex");
    push(@latexfiles, "$legal-tab.tex");
    print TAB "% tabular for $nick\n";
    print TAB "% $full\n";
    print TAB "\\FIXME[tabular for $nick would go here]\n";
    print TAB "% eof\n";
    close(TAB);
    return("intro to task table for $nick\n");
}

# domains or things get tabbings of kids
sub latex_table_kids {
    my ($key,$nick,$legal,$type) = @_;
    my $full = $wbs{$key}{'domain'};
    $full .= ' : ' . $wbs{$key}{'thing'};
    $full .= ' : ' . $wbs{$key}{'task'};
    open(TAB, ">$outputdir/$legal-tab.tex");
    push(@latexfiles, "$legal-tab.tex");
    print TAB "% tabular for $nick\n";
    print TAB "% $full\n";
    print TAB "\\FIXME[tabular for $nick would go here]\n";
    print TAB "% eof\n";
    close(TAB);
    return("intro to task table for thing or domain $nick\n");
}

# switch table generator based on type
sub latex_table {
    my ($key,$nick,$legal,$type) = @_;
    return('') if ($latextables == 0);
    if ($type eq 'task') {
        return(&latex_table_task(@_));
    } else {
        return(&latex_table_kids(@_));
    }
    return("% error\n");
}

#
# Generate text and (optionally) figures and tables for the section.
#
#my $description;
#format SECN =
#  @<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<
#$description
#^<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<
#$description
#.
sub latex_section {
    my ($key,$nick,$legal,$section,$type,$kidtype) = @_;
    my (@kids,$ks,$figtext,$tabtext,$flotext,$plural,$description);
    open(SECN, ">$outputdir/$legal.tex");
    push(@latexfiles, $legal);
    print SECN "% Material for $section on $nick\n";
    print SECN "\\$section\{$wbs{$key}{$type}\}\n";
    print SECN "This $section covers the $type with nickname ``$nick''\n";
    print SECN "as it was defined beginning with line $wbs{$key}{'line'}\n";
    print SECN "of the input file.\n";
    if ($kidtype ne '') {
        @kids = split(/,/,$wbs{$key}{'kids'});
        if ($#kids > 0) {
            # the Oxford comma
            $kids[$#kids] = 'and ' . $kids[$#kids];
            $ks = join(', ',@kids);
            print SECN "It includes (nicknames) $ks as $kidtype" . "s.\n";
            $plural = 's';
        } elsif ($#kids == 0) {
            print SECN "It includes one $kidtype, $kids[0]." . "\n";
            $plural = '';
        } else {
            # else no kids
            $plural = '';
        }
        print SECN "These $kidtype$plural will be described\n";
        print SECN "in subsequent sub\{$section\}$plural.\n";
        print SECN "The general description of the work is as follows:\n";
    } else {
        print SECN "This $section covers the work of a task as follows:\n";
    }
    $description = $wbs{$key}{'desc'};
    print SECN '%desc' . "\n\\textit\{" . $description . ".\}\n";
    print SECN "\n";
    # these generate text intro, input and and following text
    # the input file is written to $outputdir/$legal-*.tex
    if ($type ne 'task') {
        $figtext = &latex_figure($key,$nick,$legal,$type);
        print SECN "$figtext\n";
        $flotext = &latex_flows($key,$nick,$legal,$type);
        print SECN "$flotext\n";
    }
    $tabtext = &latex_table($key,$nick,$legal,$type);
    print SECN "$tabtext\n";
    print SECN "% eof\n";
    close(SECN);
}

# It is easiest to walk the WBS by uid order, making things and putting
# them on the list to connect up later into the parts of the output doc
# every call to latex_section, generates the needed files and adds them
# to @latexfiles
sub generate_lists {
    my ($orderef,$item,$nick,$legal) = (\&by_uid);
    for $item (sort($orderef keys(%wbs))) {
        $nick = $wbs{$item}{'nick'};
        # latex and dot have the similar name conventions
        $legal = &make_it_legal($nick);
        print "Latex for: $nick -> $legal*.tex\n" if ($veryverb);
        if ($wbs{$item}{'type'} eq 'domain') {
            &latex_section($item,$nick,$legal,'section','domain','thing');
        } elsif ($wbs{$item}{'type'} eq 'thing') {
            &latex_section($item,$nick,$legal,'subsection','thing','task');
        } else { # it is a task
            &latex_section($item,$nick,$legal,'subsubsection','task','');
        }
    }
}

# This makes an empty include document
sub empty_latex_input {
    print "Generating Empty Latex Main...\n" if ($verb);
    open(EMPTY, ">$outputdir/$latexfile");
    print EMPTY "% nothing in this file\n";
    close(EMPTY);
}

# this walks the list of files and makes the file to be included
# in some document template
sub full_latex_input {
    print "Generating Populated Latex Main...\n" if ($verb);
    open(LATEX, ">$outputdir/$latexfile");
    print LATEX "% this file contains a list of files included\n";
    print LATEX "% in the appropriate order to make sections,\n";
    print LATEX "% figures and tables as requested.\n\n";
    for my $lf (@latexfiles) {
        print LATEX "\\input\{$lf\}\n";
    }
    print LATEX "% eof\n";
    close(LATEX);
}

#
# main entry follows:
#   generate lists walks the WBS, making section text,
#   figures and tables as requested on a per-item basis.
#   The (ordered) list of such products then populates the input
#   to be given to latex.
#
sub generate_inputs {
    print "Generating Latex Inputs...\n" if ($verb);
    if ($latexinputs == 0) {
        # generate a stub so that the document builds
        print "Generating Latex Input Stub\n";
        &empty_latex_input();
        return;
    }
    # walk the WBS generating tex files to be \input'd
    &generate_lists();
    # walk the list of tex files and build the main input
    &full_latex_input();
}

# this is a library module, so it needs to return TRUE.
1;
#
# eof
#
