#!/usr/bin/perl
#
use warnings;           # turns on optional warnings
use diagnostics;        # and makes them not terse
use strict;             # makes unsafe constructs illegal
#
# Support script for walking through the wbs
#

our @keywords;
our %domains;
our %things;
our %tasks;
our %wbs;
our $sep;

our $sort_key;
our $verb;
our $veryverb;

# some subsets
my @jelly   = ( 'nick', 'start', 'stop', 'days', 'who',
                'task', 'thing', 'domain', 'desc' );
my @summary = ( 'desc', 'nick', 'who', 'stop' );
my @simple  = ( 'desc' );
# this is for the NEW file, but 'file' must appear first
my @inputs  = ( 'nick', 'desc', 'path', ,'code', 'notes', 'level',
                'issues', 'orient',
                'who', 'needs', 'start', 'days', 'derate', 'done',
                'stop', 'allows' );

# trying to use forms
our ($fkey,$fval);
format WBS =
  @>>>>>>>>  ^<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<
  $fkey,     $fval
~~           ^<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<
             $fval
.
format NEW =
@<<<<<<<<  = ^<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<
  $fkey,     $fval
~~           ^<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<
             $fval
.

#
# convert a mnemonic into a list
#
sub report_on {
    my $what = $_[0];
    if ($what eq 'all') { return(@keywords); }
    elsif ($what eq 'sum') { return(@summary); }
    elsif ($what eq 'bean') { return(@jelly); }
    elsif ($what eq 'what') { return(@simple); }
    # else
    print "Undefined report $what so you just get a simple list.\n";
    return(@simple);
}

#
# a comparison on %wbs for sort
#
sub by_uid { return( $wbs{$a}{'uid'} <=> $wbs{$b}{'uid'} ); }
sub by_begin { return( $wbs{$a}{'begin'} <=> $wbs{$b}{'begin'} ); }
sub by_key {
    return &by_uid   if ($sort_key eq 'uid');
    return &by_begin if ($sort_key eq 'begin');
    if (defined($wbs{$a}{$sort_key}) and
        defined($wbs{$b}{$sort_key})) {
        return( $wbs{$a}{$sort_key} cmp $wbs{$b}{$sort_key} );
    }
    if (!defined($wbs{$a}{'uid'})) { print "Undef uid on $a $sort_key\n"; }
    if (!defined($wbs{$b}{'uid'})) { print "Undef uid on $b $sort_key\n"; }
    return( $wbs{$a}{'uid'} <=> $wbs{$b}{'uid'} );
}

#
# helper function; remember that keys are
#   domain . sep . thing . sep . task
#
sub write_new_rest {
    my ($key,$k,$kv,@p) = @_;
    @p = split(/$sep/,$key);
    $kv = $p[0];
    if (defined $wbs{$key}{'file'}) {
        $fval = $wbs{$key}{'file'};
        $fkey = 'file';
        write NEW if ($fval ne '');
    }
    while ($#p >= 0) {
        $fval = shift(@p);
        $fkey = $wbs{$kv}{'type'};
        $kv .= $sep . $p[0] if ($#p >= 0);
        write NEW;
    }
    # dump the defined remainder of it
    for $k (@inputs) {
        $fkey = $k;
        $fval = $wbs{$key}{$k};
        write NEW if ($fval ne '');
    }
    print NEW "\n";
}

#
# dump the entire structure in the input format
#
sub dump_new_input {
    my ($out,$in,$t) = @_;
    open(NEW,$out);
    print NEW "#\n# Generated from $in\n#\n\n";
    print NEW "HEAD = start reading here\n\n";
    for $t (keys(%domains)) { &write_new_rest($t); }
    for $t (keys(%things)) { &write_new_rest($t); }
    for $t (keys(%tasks)) { &write_new_rest($t); }
    print NEW "\nTAIL = stop reading here\n\n";
    print NEW "#\n# eof\n#\n";
    close(NEW);
}

#
# dump out the entire structure
#
sub dump_the_wbs {
    my ($out,$orderef,@wbskeys) = @_;
    my ($task,$key);
    open(WBS,$out);
    if ($veryverb) {
        print "# Reporting on: ";
        for my $k (@wbskeys) { print "$k, "; } print "\n";
    }
    for $task (sort($orderef keys(%wbs))) {
        print WBS $wbs{$task}{'type'} . " $task\n";
        for $key (@wbskeys) {
            if (defined($wbs{$task}{$key})) {
                print WBS ' ' . $key . ' -> ' . $wbs{$task}{$key} . "\n";
            }
        }
        print WBS "\n";
    }
    close(WBS);
}

#
# same thing, but just the domain/thing/task
#
sub dump_items {
    my ($what,$out,$orderef,@wbskeys) = @_;
    my ($task,$key);
    open(WBS,$out);
    for $task (sort($orderef keys(%wbs))) {
        next if ($wbs{$task}{'type'} ne $what);
        print WBS $wbs{$task}{'type'} . " $task\n";
        for $key (@wbskeys) {
            if (defined($wbs{$task}{$key})) {
                $fkey = $key;
                $fval = $wbs{$task}{$key};
                write WBS;
            }
        }
        print WBS "\n";
    }
    close(WBS);
}

sub dump_the_domains { &dump_items('domain',@_); }
sub dump_the_things { &dump_items('thing',@_); }
sub dump_the_tasks { &dump_items('task',@_); }

sub dump_tsv_tasks {
    my ($out,$orderef,@wbskeys) = @_;
    my ($aa,$bb,$cc,$preds);
    my ($task,$key);
    open(TSV,$out);
    for $task (sort($orderef keys(%wbs))) {
        next if ($wbs{$task}{'type'} ne 'task');
        print TSV '#' . " $task\n" if ($verb);
        $aa = int($wbs{$task}{'uid'} / 100000 + 0.1);
        $bb = int($wbs{$task}{'uid'} /   1000 + 0.1) - 100 * $aa;
        $cc = $wbs{$task}{'uid'} - $aa * 100000 - $bb * 1000;
        print TSV $aa . "\t" . $bb . "\t" . $cc . "\t";
        print TSV $wbs{$task}{'uid'};
        for $key (@wbskeys) {
            if (defined($wbs{$task}{$key})) {
                $_ = $wbs{$task}{$key};
                s/\n/ /;
                print TSV "\t" . $wbs{$task}{$key};
            }
        }
        $preds = &pred_uids("\t",$wbs{$task}{'needs'});
        print TSV $preds . "\n";
    }
    close(TSV);
}

#
# something to generate a list of predecessors by uid
#
sub pred_uids {
    my ($delim,$list,$reply,@parts) = @_;
    @parts = split(/,/,$list);
    $reply = '';
    for my $need (@parts) {
        $reply .= "\t" . $wbs{&task_by_nick($need,'pred_uids')}{'uid'};
    }
    return($reply);
}

#
# go through and nuke entries with any of input: derived: either:
#
sub canonicalize_items {
    for my $task (keys(%wbs)) {
        for my $key (@keywords) {
            if (not defined($wbs{$task}{$key}) or
                $wbs{$task}{$key} =~ m/input:.*/ or
                $wbs{$task}{$key} =~ m/derived:.*/ or
                $wbs{$task}{$key} =~ m/either:.*/) {
                # a few require numerical values
                if ($key eq 'days') {
                    $wbs{$task}{$key} = 0;
                } elsif ($key eq 'done') {
                    $wbs{$task}{$key} = 0;
                } elsif ($key eq 'fte') {
                    $wbs{$task}{$key} = 1;
                } elsif ($key eq 'derate') {
                    $wbs{$task}{$key} = 3;
                } else {
                    $wbs{$task}{$key} = '';
                }
            }
        }
    }
}

#
# a simple lookup function for use as needed
#
sub task_by_nick {
    my ($nick,$caller,$kv) = @_;
    $caller = 'undefined' if (not defined $caller);
    for $kv (keys(%wbs)) {
        next if (!defined($wbs{$kv}{'nick'}));
        if ($wbs{$kv}{'nick'} eq $nick) {
            return($kv);
        }
    }
    print "No task for '$nick' ($caller), returning 'none'.\n" if ($verb);
    return('none');
}

#
# assign kids to every thing and domain
#
sub make_kids_of_things {
    my ($kv,$nick,$pop);
    for $kv (keys(%tasks)) {
        $nick = $wbs{$kv}{'nick'};
        $pop = $wbs{$kv}{'parent'};
        if ($wbs{$pop}{'kids'} eq '') {
            $wbs{$pop}{'kids'} = $nick;
        } else {
            $wbs{$pop}{'kids'} .= ',' . $nick;
        }
    }
}
sub make_kids_of_domains {
    my ($kv,$nick,$pop);
    for $kv (keys(%things)) {
        $nick = $wbs{$kv}{'nick'};
        $pop = $wbs{$kv}{'parent'};
        if ($wbs{$pop}{'kids'} eq '') {
            $wbs{$pop}{'kids'} = $nick;
        } else {
            $wbs{$pop}{'kids'} .= ',' . $nick;
        }
    }
}

#
# this is a library module, so it needs to return TRUE.
#
1;
#
# eof
#
