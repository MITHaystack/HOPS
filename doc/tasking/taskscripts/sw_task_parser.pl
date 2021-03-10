#!/usr/bin/perl
#
use warnings;           # turns on optional warnings
use diagnostics;        # and makes them not terse
use strict;             # makes unsafe constructs illegal
#
# Support script for parsing the sw task data file
#

# hashes of domains, things, and tasks
our %domains;
our %things;
our %tasks;
# the work breakdown structure
our %wbs;
our $sep = '%%';
# the files in use
our %files;
# verbose for feedback, veryverb for debugging
our $output;
our $verb;
our $veryverb;

#
# first mention of domain/thing/task defines it
# next mention merely establishes parentage
# ^@* = value       to define abbreviations (substituted thereafter)
#                   Note: abbreviations automatically made for nick's.
# ^tdefs = name     allows @name later to fill out task entries
#                   definition ends with blank or comment line
#
# the keywords define the input language
# and must be initialized in taskage.
#
our @keywords = (
    'type', 'domain', 'thing', 'task',
    'desc', 'nick', 'parent', 'kids', 'line',
    'path', ,'code', 'docref', 'notes', 'level', 'issues', 'orient',
    'who', 'pri', 'sec', 'men', 'fte',
    'uid', 'needs', 'start', 'begin', 'days', 'derate',
    'done', 'mjds', 'flex', 'end', 'stop', 'allows',
    'preps', 'leads', 'file'
);

my %taskage = (
    # what it is
    'type'   => 'derived: one of domain, thing, task',
    'domain' => 'derived: some domain of activity',
    'thing'  => 'derived: some thing in domain to be done',
    'task'   => 'derived: tasks to required complete thing',

    # basic detail about the task
    'desc'   => 'input: short description of task/thing/domain',
    'nick'   => 'input: nickname for task/thing/domain',
    'parent' => 'derived: wbs | thing of task | or domain of thing',
    'kids'   => 'derived: domains or things',
    'line'   => 'derived: line number in original file',

    # yet more detail about the task
    'path'   => 'input: CVS:where',
    'code'   => 'input: Bash/C/C++/Java/Python, &c.',
    'notes'  => 'input: mention some local file with more details',
    'docref' => 'input: Documentation reference, doc, section, page, &c',
    'level'  => 'input: easy | moderate | difficult',
    'issues' => 'input: open issues relevant to planning',
    'orient' => 'input: portrait/landscape',
    'attr'   => 'input: node attributes for dot plots',

    # workers associated with task
    'who'    => 'input: list of workers',
    'pri'    => 'derived: primary (1st in who)',
    'sec'    => 'derived: secondary (2nd in who)',
    'men'    => 'derived: everyone else',
    'fte'    => 'derived: total fte (.66pri,.33sec,0else)',

    # dependencies and timeline
    'uid'    => 'derived: ordinal identifier',
    'needs'  => 'input: ,-sep list of predecessor nicks',
    'start'  => 'either: start date if days=0, else derived',
    'days'   => 'input: estimate of real man-days of work',
    'derate' => 'input: multiplier on ftes to get calendar days',
    'begin'  => 'derived: MJD of start date',
    'done'   => 'input: percentage done',
    'mjds'   => 'derived: calendar days required',
    'flex'   => 'derived: calendar days of margin',
    'end'    => 'derived: MJD of finish date',
    'stop'   => 'either: finish date if days=0, else derived',
    'allows' => 'input: ,-sep list of successor nicks',
    'preps'  => 'derived: nick that defines start of this one',
    'leads'  => 'derived: nick that defines stop of this one',

    # for multiple developers
    'file'   => 'input: source file for the component',
);

# a hash of task defaults
my %taskdefs = ();
# a hash of abbreviations
my %abbrevs = ();

# private parsing variables
my $current_taskdef = '';
my $current_domain = '';
my $current_thing = '';
my $current_task = '';
my $current_item = '';
my $current_file = 'no-such-file.txt';
my ($domcntr,$thgcntr,$tskcntr) = (1,1,1);

#
# This is a function to print out the internal keyword help
# report is none,all,sum,what,help
#
our ($fkey, $fval);
format = 
  @>>>>>>>> = ^<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<
  $fkey,     $fval
.
sub dump_taskage {
    print "\n";
    print "The -r report options specify the set of keywords\n";
    print "that will be dumped to the overall .wbs, .domain, .thing,\n";
    print ".task or .tsv files.  The choices are:\n";
    print "  none  you none of these files (the default)\n";
    print "  what  gets you the description\n";
    print "  sum   desc, nick, who, and stop\n";
    print "  bean  sum items, and nick, start, days, task, think and domain\n";
    print "  all   gets you all known keywords\n";
    print "\n";
    print "The complete (sorted) list of keywords follows.  In the\n";
    print "descriptions, the prefixes 'derived', 'input', or 'either'\n";
    print "give you some idea of whether the item comes in through the\n";
    print "input file, or is calculated from other information.\n";
    for $fkey (sort(keys(%taskage))) {
        $fval = $taskage{$fkey};
        write;
    }
    print "\n";

}

#
# Undo abbreviations -- replace @foo by foo's value.
# Note that spaces around the abbreviation are not required.
#
sub deabbr {
    #my ($_) = @_;
    local ($_) = @_;
    for my $key (keys(%abbrevs)) { s($key)($abbrevs{$key}); }
    for my $key (keys(%abbrevs)) { s($key)($abbrevs{$key}); }
    return $_;
}

#
# Make an abbreviation by stripping off wrapper spaces
#
sub makeabbr {
    #my ($_,$ln,@bits) = @_;
    local $_; my ($ln,@bits); ($_,$ln) = @_;
    # toss # through end of line
    s{#.*}{};
    @bits = split(/=/);
    $bits[0] =~ s{([@]\S+)\s*}{$1};
    $bits[1] =~ s{\s*(.*\S+)\s*}{$1};
    $abbrevs{$bits[0]} = &deabbr($bits[1]);
    print DEBUG "# abbrev  " . $bits[0] . "->" .
            $abbrevs{$bits[0]} . "<\n" if ($veryverb);
    return $_ . "\n";
};


#
# some functions to create a new object with defaults for
# everything and to then hook it up to what is already known.
#
# canonicalize_items() will eventually overwrite things left undefined
#
sub create_object {
    my $kv = $_[0];
    $wbs{$kv} = ();
    for my $key (keys(%taskage)) {
        $wbs{$kv}{$key} = $taskage{$key};
    }
}
sub set_default_object {
    my ($kv,$ln,$type,$parent,@parts) = @_;
    &create_object($kv);
    $wbs{$kv}{'type'} = $type;
    $wbs{$kv}{'parent'} = $parent;
    $wbs{$kv}{'kids'} = '';
    $wbs{$kv}{'issues'} = '';
    $wbs{$kv}{'orient'} = 'portrait';
    $wbs{$kv}{'line'} = $ln;
    $wbs{$kv}{'nick'} = $kv;
    $wbs{$kv}{'file'} = $current_file;
#   $wbs{$kv}{'uid'} = $uidcntr++;
#   for my $ii ('domain','thing','task') {
#       $wbs{$kv}{$ii} = ($ii eq $type) ? 'true' : 'false';
#   }
    @parts = split(/$sep/,$kv);
    if ($type eq 'domain') {
        $wbs{$kv}{'domain'} = $parts[0];
        $wbs{$kv}{'thing'} = 'none';
        $wbs{$kv}{'task'} = 'none';
        $wbs{$kv}{'uid'} = ($domcntr++) * 100000;
    } elsif ($type eq 'thing') {
        $wbs{$kv}{'domain'} = $parts[0];
        $wbs{$kv}{'thing'} = $parts[1];
        $wbs{$kv}{'task'} = 'none';
        $wbs{$kv}{'uid'} = ($thgcntr++) * 1000;
        $wbs{$kv}{'uid'} += $wbs{$parent}{'uid'};
    } elsif ($type eq 'task') {
        $wbs{$kv}{'domain'} = $parts[0];
        $wbs{$kv}{'thing'} = $parts[1];
        $wbs{$kv}{'task'} = $parts[2];
        $wbs{$kv}{'uid'} = $tskcntr++;
        $wbs{$kv}{'uid'} += $wbs{$parent}{'uid'};
    }
}

#
# Parse one line of the input: called with the line ($_) and number ($ln)
#
sub really_parse_one_line {
    #my ($_,$ln,$kv,@bits) = @_;
    #local ($_); my ($ln,$kv,@bits); ($_,$ln,$kv,@bits) = @_;
    local ($_); my ($ln,$kv,@bits); ($_,$ln) = @_;
    # toss #.* through the end of the line
    s{#.*}{};
    @bits = split(/=/);
    # make sure there is a value, even if empty
    push @bits,'';
    # the value on the line, surrounding white-space removed
    $bits[1] =~ s{\s*(.*\S+)\s*}{$1};
    # now with abbreviations expanded
    $kv = &deabbr($bits[1]);
    print DEBUG '## line ' . $ln . ':' . $_ if ($veryverb);

    # $current_item is the key with surrounding white-space removed
    $current_item = $bits[0];
    $current_item =~ s{^\s*}{};
    $current_item =~ s{\s*$}{};


    # tdefs handling
    if ( /^tdefs/ ) {
        $current_taskdef = $kv = '@' . $kv;
        $taskdefs{$kv} = '';
    } elsif ( $current_taskdef ne '' ) {
        $taskdefs{$current_taskdef} .= &deabbr($_) . "%%%";

    # what kind of line is it -- do appropriate processing
    } elsif ( /^domain/ ) {
        if (!defined($domains{$kv})) {
            $domains{$kv} = $ln;
            &set_default_object($kv,$ln,'domain','wbs');
        }
        $current_task = $current_thing = $current_domain = $kv;
    } elsif ( /^thing/  ) {
        $kv = $current_domain . $sep . $kv;
        if (!defined($things{$kv})) {
            $things{$kv} = $ln;
            &set_default_object($kv,$ln,'thing',$current_domain);
            if (defined($wbs{$current_domain}{'nick'})) {
                ($wbs{$kv}{'nick'} = $wbs{$current_domain}{'nick'} .
                    '-' . $bits[1]) =~ s/[@ ]//g;
            }
        }
        $current_task = $current_thing = $kv;
    } elsif ( /^task/  ) {
        $kv = $current_thing . $sep . $kv;
        if (!defined($tasks{$kv})) {
            $tasks{$kv} = $ln;
            &set_default_object($kv,$ln,'task',$current_thing);
            if (defined($wbs{$current_thing}{'nick'})) {
                ($wbs{$kv}{'nick'} = $wbs{$current_thing}{'nick'} .
                    '-' . $bits[1]) =~ s/[@ ]//g;
            }
        }
        $current_task = $kv;
    } elsif ( /^file/ ) {
        $current_file = $kv;

    } elsif ( /^line/ or /^parent/ or /^type/ or /^kids/) {
        ; # derived

    } elsif ( /^desc/ ) {
        $wbs{$current_task}{'desc'} = $kv;
        # $current_item = 'desc';
    } elsif ( /^nick/ ) {
        #&makeabbr('@' . $kv . ' = ' .  $wbs{$current_task}{'nick'},$ln);
        @bits = split(/$sep/,$current_task);
        &makeabbr('@' . $kv . ' = ' . $bits[$#bits],$ln);
        $wbs{$current_task}{'nick'} = $kv;

    } elsif ( /^path/ ) {
        $wbs{$current_task}{'path'} = $kv;
    } elsif ( /^code/ ) {
        $wbs{$current_task}{'code'} = $kv;
    } elsif ( /^notes/ ) {
        $wbs{$current_task}{'notes'} = $kv;
    } elsif ( /^days/ ) {
        $wbs{$current_task}{'days'} = $kv;
    } elsif ( /^derate/ ) {
        $wbs{$current_task}{'derate'} = $kv;
    } elsif ( /^level/ ) {
        $wbs{$current_task}{'level'} = $kv;
    } elsif ( /^issues/ ) {
        $wbs{$current_task}{'issues'} = $kv;
    } elsif ( /^orient/ ) {
        $wbs{$current_task}{'orient'} = $kv;

    } elsif ( /^who/ ) {
        $wbs{$current_task}{'who'} = $kv;
        my @people = split(/,/,$kv,3);
        $wbs{$current_task}{'pri'} = $people[0];    # first is primary
        $wbs{$current_task}{'sec'} = $people[1];    # second is secondary
        $wbs{$current_task}{'men'} = $people[2];    # everyone else

    } elsif ( /^needs/ ) {
        $wbs{$current_task}{'needs'} = $kv;
    } elsif ( /^start/ ) {
        $wbs{$current_task}{'start'} = $kv;
    } elsif ( /^done/ ) {
        $wbs{$current_task}{'done'} = $kv;
    } elsif ( /^stop/ ) {
        $wbs{$current_task}{'stop'} = $kv;
    } elsif ( /^allows/ ) {
        $wbs{$current_task}{'allows'} = $kv;
    } elsif ( /^uid/ or /^begin/ or /^mjds/ or /^flex/ or /^end/ ) {
        ; # derived

    }
    return $_ . "\n";
}

#
# most things can be continued with whitespace in the first column
#
sub continued_item {
    #my ($_,$ln,$strip) = @_;
    local ($_); my ($ln,$strip); ($_,$ln,$strip) = @_;
    $strip = $_;
    $strip =~ s/\s*(.*)\s*/$1/;
    $strip = &deabbr($strip);
    if (defined($wbs{$current_task}{$current_item})) {
        if ($wbs{$current_task}{$current_item} =~ m/^.*-$/) {
            $wbs{$current_task}{$current_item} .= $strip;
        } else {
            $wbs{$current_task}{$current_item} .= ' ' . $strip;
        }
    }
    return $_ . "\n";
}

#
# If this task is defined, then step through its definition, expanding it.
#
sub insert_taskdef {
    #my ($_,$ln,@bits) = @_;
    local ($_); my ($ln,@bits); ($_,$ln,@bits) = @_;
    print DEBUG "# taskdef " . $_ . "\n" if ($veryverb);
    if (defined($taskdefs{$_})) {
        @bits = split(/%%%/,$taskdefs{$_});
        $_ = '# ' . $_ . ' at line ' . $ln . ":\n";
        for my $b (@bits) { $_ .= &really_parse_one_line($b,$ln); }
    }
    return $_;
}

#
# taskdefs end with a blank line or comment
#
sub finish_taskdef {
    $current_taskdef = '';
    return $_ . "\n";
}

#
# switch to handle @definedtask
#
sub parse_one_line {
    #my ($_,$ln,@bits) = @_;
    local ($_); my ($ln,@bits); ($_,$ln,@bits) = @_;
    if ( /^@.*=/ )  {       return(&makeabbr($_,$.)); }
    elsif ( /^@/ )  { return(&insert_taskdef($_,$.)); }
    elsif ( /^$/ )  { return(&finish_taskdef($_,$.)); }
    elsif ( /^#/ )  { return(&finish_taskdef($_,$.)); }
    elsif ( /^\s/ ) { return(&continued_item($_,$.)); }
    else {    return(&really_parse_one_line($_,$.)); }
    return "#never get here\n";
}

#
# Parse the provided file and generate the data structures
# Returns the three pieces of the scanned file.
#
sub parse_sw_task_file {
    my ($file) = @_;
    my ($ok,$hdr,$guts,$ftr) = (0,'','','');
    $current_file = $file;
    &dump_open(">$output-parse.dbg") if ($veryverb);
    open(DATA,"<",$file);
    while (<DATA>) {
        chomp;
        if    ( /^HEAD/ ) { $ok = 1; $hdr .= $_ . "\n"; }
        elsif ( /^TAIL/ ) { $ok = 2; $ftr .= $_ . "\n"; }
        elsif ( $ok == 0 ) { $hdr .= $_ . "\n"; }
        elsif ( $ok == 2 ) { $ftr .= $_ . "\n"; }
        else  { $guts .= &parse_one_line($_,$.); }
    }
    &dump_close() if ($veryverb);
    close(DATA);
    return($hdr,$guts,$ftr);
}

#
# For debugging, dump the task defs
#
sub dump_taskdefs {
    for my $td (sort(keys(%taskdefs))) {
        print DEBUG "TDEF: $td = >" . $taskdefs{$td} . "<\n";
    }
}
sub dump_abbrevs {
    for my $td (sort(keys(%abbrevs))) {
        print DEBUG "ABBR: $td = >" . $abbrevs{$td} . "<\n";
    }
}
sub dump_open {
    open(DEBUG,$_[0]);
}
sub dump_close {
    close(DEBUG);
}

#
# this is a library module, so it needs to return TRUE.
#
1;

#
# eof
#
