#!/usr/bin/perl
#
use warnings;           # turns on optional warnings
use diagnostics;        # and makes them not terse
use strict;             # makes unsafe constructs illegal

# common controls
our ($verb,$doth2,$outputdir,%wbs);

# latex support is conditional on these:
our $latexfigures;      # generate the figures
our $latextables;       # generate the tables
our $latexgantage;      # reference UML gantt products
our $latexinputs;       # generate the input files
our $latexfile;         # name of the main file

# working variables; every domain or thing gets a file in uid sort order
my @latexfiles = ();

sub latex_figure {
    my ($key,$nick,$legal,$type) = @_;
    return('') if ($latexfigures == 0);
    return("% nothing yet for figure on $key,$nick,$legal,$type\n");
}

sub latex_table {
    my ($key,$nick,$legal,$type) = @_;
    return('') if ($latextables == 0);
    return("% nothing yet for table for figure on $key,$nick,$legal,$type\n");
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
    my (@kids,$ks,$figtext,$tabtext,$plural,$description);
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
        } # else no kids
        print SECN "These $kidtype$plural will be described\n";
        print SECN "in subsequent sub\{$section\}$plural.\n";
        print SECN "The general description of the work is as follows:\n";
    } else {
        print SECN "This $section covers the work of a task as follows:\n";
    }
    $description = $wbs{$key}{'desc'};
    print SECN '%desc' . "\n\\textit\{" . $description . ".\}\n";
    print SECN "\n";
    $figtext = &latex_figure($key,$nick,$legal,$type);
    print SECN "$figtext\n";
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
        $legal = &make_it_legal($nick);
        print "Latex for: $nick -> $nick-*.tex\n"; # if ($veryverb);
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
        print LATEX "\\input\{$lf\}\n\n";
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
        print "Generating Latex Input Stub\n";
        &empty_latex_input();
        return;
    }
    &generate_lists();
    &full_latex_input();
}

# this is a library module, so it needs to return TRUE.
1;
#
# eof
#
