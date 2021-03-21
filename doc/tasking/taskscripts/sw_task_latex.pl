#!/usr/bin/perl
#
use warnings;           # turns on optional warnings
use diagnostics;        # and makes them not terse
use strict;             # makes unsafe constructs illegal

# common controls
our ($verb,$veryverb,$doth2,%wbs);
# $outputdir is the directory; $output is the directory/prefix
our ($outputdir,$output,$dotdev);

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

# a generic figure float generator: given short caption title,
# full caption text, graphics file and nickname, return enough latex
# to generate the float and the text of the label to reference.
sub create_figure_float {
    my ($short,$fullcap,$gname,$wm,$hm) = @_;
    my $pcbar = '%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%';
    my $ref     = 'fig:' . $short;
    my $gibber  = 'width='.$wm.'\textwidth';
    $gibber    .= ',height='.$hm.'\textheight' if ($hm > 0);
    $gibber    .= ',keepaspectratio';
    my $floater = $pcbar . "\n";
    $floater .= '\begin{figure}[htbp]' . "\n";
    $floater .= '\captionsetup{width=0.6\linewidth}' . "\n";
    $floater .= '\center{\fbox{%' . "\n";
    $floater .= '\includegraphics['.$gibber.']{' . $gname . "}}}\n";
    $floater .= '\caption[' . $short . ']{' . $fullcap . "}\n";
    $floater .= '\label{' . $ref . "}\n";
    $floater .= '\end{figure}' . "\n";
    $floater .= $pcbar . "\n";
    return($ref, $floater);
}

sub latex_flows {
    my ($key,$nick,$legal,$type) = @_;
    my ($rc,$arg,$captext,$gname,$ref,$fig,@args);
    my $full = $wbs{$key}{'domain'};
    $full .= ' : ' . $wbs{$key}{'thing'};
    $full .= ' : ' . $wbs{$key}{'task'};
    return('') if ($latexflowcharts == 0);
    open(FLO, ">$outputdir/$legal-flo.tex");
    push(@latexfiles, "$legal-flo.tex");
    $arg = $flowcharterargs;
    $arg =~ s/OUTPUT/$output/;
    $arg =~ s/NICK/$nick/;
    $arg =~ s/DOTDEV/$dotdev/;
    @args = split(/ /,$arg);
    unshift(@args, "./$flowcharter");
    print FLO "% figure for flow (Gantt) chart for $nick\n";
    print FLO "% $full\n";
    $rc = system(@args);
    print FLO "% via system(" . join(',',@args) . ") = $rc.\n";
    $gname = "$output-$nick-flo";
    if (-f $gname . ".$dotdev") {
        $captext = "This is a flow or Gantt chart representing the work\n";
        $captext .= "to be carried out in this $wbs{$key}{'type'}.";
        ($ref,$fig) = &create_figure_float($nick,$captext,$gname,0.9,0.0);
        print FLO $fig;
    } else {
        $ref = 'fig:failed-flow' . $nick;
        print FLO "\\FIXME[flow (Gantt) chart generation for $nick failed]\n";
    }
    print FLO "\n% eof\n";
    close(FLO);
    return("The flow (Gantt) chart for $nick is displayed in\n" .
           "Fig.~\\ref{$ref}.  Dates are calculated assuming\n" .
           "one FTE of effort.\n");
}

# include the generated figure graphic in a figure float with caption
sub latex_figure {
    my ($key,$nick,$legal,$type) = @_;
    my ($ref,$fig,$captext,$gname);
    my $full = $wbs{$key}{'domain'};
    $full .= ' : ' . $wbs{$key}{'thing'};
    $full .= ' : ' . $wbs{$key}{'task'};
    return('') if ($latexfigures == 0);
    open(FIG, ">$outputdir/$legal-fig.tex");
    push(@latexfiles, "$legal-fig.tex");
    print FIG "% figure for $nick\n";
    print FIG "% $full\n";
    $gname = "$output-$nick";
    if (-f $gname . ".$dotdev") {
        $captext = "This is the \\ac{TBM} diagram for $type $nick.";
        ($ref,$fig) = &create_figure_float($nick,$captext,$gname,0.9,0.9);
        print FIG $fig;
    } else {
        print FIG "\\FIXME[figure for $nick would go here]\n";
    }
    print FIG "\n% eof\n";
    close(FIG);
    return("The \\ac{TBM} diagram for $type $nick is shown in\n" .
           "Fig.~\\ref{$ref}.\n");
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
    print TAB "\n% eof\n";
    close(TAB);
    return("Intro to task table for $nick\n");
}

# generate tabular text for a domain or thing
# start dates for domains are not useful, so we skip that column
sub tabular_domain_thing {
    my ($key,$nick,$type,$rs,$kid,$kt,$what,$npbrk) = @_;
    my @kids = split(/,/,$wbs{$key}{'kids'});
    $type = $wbs{$key}{'type'};
    # demote $type to be that of the kids.
    if ($type eq 'domain') { $type = 'thing'; }
    else                   { $type = 'task'; }
    $rs .= '\small' . "\n";
    $rs .= "\\begin{tabbing}\n";
    $rs .= 'YYYY-MM-DD \=' if ($type eq 'task');
    $rs .= 'xxxxxxxxxxxxxxxxxxxxxxxx \=';
    $rs .= 'xxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx';
    $rs .= "\\kill\n";
    $what = 'Thing' if ($type eq 'thing');
    $what = 'Task' if ($type eq 'task');
    $rs .= "\\textbf{Start} \\>" if ($type eq 'task');
    $rs .= "\\textbf{$what Nickname} ";
    $rs .= " \\> \\textbf{Full Name}";
    $rs .= "\\nopagebreak\\\\\n";
    $rs .= "\\rule{0.90\\textwidth}{\\arrayrulewidth}\\nopagebreak\\\\\n";
    $npbrk = '\nopagebreak';
    for $kid (@kids) {
        $kt = &task_by_nick($kid,'tabular_domain');
        $rs .= $wbs{$kt}{'start'} . '\>' if ($type eq 'task');
        $rs .= $kid . '\>' . $wbs{$kt}{$type};
        $rs .= $npbrk . "\\\\\n";
        $npbrk = '';    # force header and first line on same page
    }
    $rs .= "\\end{tabbing}\n";
    $rs .= '\normalsize' . "\n";
    return($rs);
}

# domains or things get tabbings of kids
sub latex_table_kids {
    my ($key,$nick,$legal,$type) = @_;
    my $full = $wbs{$key}{'domain'};
    $full .= ' : ' . $wbs{$key}{'thing'};
    $full .= ' : ' . $wbs{$key}{'task'};
    open(TAB, ">$outputdir/$legal-tab.tex");
    push(@latexfiles, "$legal-tab.tex");
    print TAB "% tabular for $type $nick\n";
    print TAB "% $full\n";
    print TAB &tabular_domain_thing($key,$nick);
    print TAB "\n% eof\n";
    close(TAB);
    return("The $type $nick is more described in detail in the\n" .
           "following sections. For reference, here is a short list.\n");
}

# switch the table generator based on type
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
# Domains get fresh page and a flush of any pending floats.
#
sub latex_section {
    my ($key,$nick,$legal,$section,$type,$kidtype) = @_;
    my (@kids,$ks,$figtext,$tabtext,$flotext,$plural,$description);
    open(SECN, ">$outputdir/$legal.tex");
    push(@latexfiles, $legal);
    print SECN "% Material for $section on $nick\n";
    print SECN "% clearing floats\n\\clearpage\n" if ($section eq 'section');
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
    print SECN "\n% eof\n";
    close(SECN);
}

# It is easiest to walk the WBS by uid order, making things and putting
# them on the list to connect up later into the parts of the output doc
# every call to latex_section (with a few sugary words), generates the
# needed files and adds them to @latexfiles.
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

# Walk the list of files and generate a series of inclusions
sub full_latex_input {
    print "Generating Populated Latex Main...\n" if ($verb);
    open(LATEX, ">$outputdir/$latexfile");
    print LATEX "% this file contains a list of files included\n";
    print LATEX "% in the appropriate order to make sections,\n";
    print LATEX "% figures and tables as requested.\n\n";
    for my $lf (@latexfiles) {
        print LATEX "\\input\{$lf\}\n";
    }
    print LATEX "\n% eof\n";
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
