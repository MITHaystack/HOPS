#!/usr/bin/perl
#
use warnings;           # turns on optional warnings
use diagnostics;        # and makes them not terse
use strict;             # makes unsafe constructs illegal
#
# Calendar dates are of the form
# YYYY-MM-DD <=> MJD (5 digits)
# 1995-10-10     50000  $gBegin (set in sw_tasks.pl, override in $config)
# 2023-02-25     60000
# 2050-07-13     70000  $gEnd   (set in sw_tasks.pl, override in $config)
# note these are calendar time dates.
our %wbs;
our ($date_pat,$mjd_pat);
our ($verb,$veryverb);
our ($gBegin,$gEnd);

# the label used for start/stop times not yet resolved
our ($depends);
# skip generation of needs / allows
our $skipneedsallows;
# allow debugging of the math to output file and limiting passes
our ($debugmjdupdates,$output,$maxtimelinepasses,$maxtimelineestimates);

#
# This routine computes begin/end from start/stop
# and assigns the time required and margin members.
# Undefined start/stop times are canonicalized to $depends.
# for the following adjustment phases.
#
# When $est is 0 we require ok=2.
#
sub update_mjd_data {
    my ($est) = @_;
    my ($ok,$gap,$mods,$m,$dr,$dys,$mjs,$cm) = (0,0,0);
    open(MJDBG,">$output" . '.mjd.dbg') if ($debugmjdupdates);
    for my $kv (keys(%wbs)) {
        $ok = 0;
        if ($wbs{$kv}{'start'} =~ m/$date_pat/) {
            $m = &date_to_mjd($wbs{$kv}{'start'});
            $mods++ if ($m ne $wbs{$kv}{'begin'});
            $wbs{$kv}{'begin'} = $m;
            $ok++;
        } else {
            $wbs{$kv}{'start'} = $depends;
            $wbs{$kv}{'begin'} = $gBegin;
            $ok = 0;
        }
        if ($wbs{$kv}{'stop'} =~ m/$date_pat/) {
            $m = &date_to_mjd($wbs{$kv}{'stop'});
            $mods++ if ($m ne $wbs{$kv}{'end'});
            $wbs{$kv}{'end'} = $m;
            $ok++;
        } else {
            $wbs{$kv}{'stop'} = $depends;
            $wbs{$kv}{'end'} = $gEnd;
            $ok = 0;
        }
        if (($ok eq 2) and ($wbs{$kv}{'days'} > 0)) {
            $gap = $wbs{$kv}{'end'} - $wbs{$kv}{'begin'};
            $wbs{$kv}{'mjds'} = $wbs{$kv}{'days'} * $wbs{$kv}{'derate'};
            $wbs{$kv}{'flex'} = $gap - $wbs{$kv}{'mjds'};
            $dr = $wbs{$kv}{'derate'};
            $cm = 'ok';
            # FIXME: 'fte'
        } elsif ($wbs{$kv}{'days'} eq 0) {    # a milestone
            $wbs{$kv}{'mjds'} = 0;
            $wbs{$kv}{'flex'} = 0;
            $wbs{$kv}{'preps'} = $wbs{$kv}{'leads'} = 'milestone';
            $dr = 0;
            $cm = 'milestone';
            # FIXME: 'fte'
        } else {
            $wbs{$kv}{'mjds'} = 0;
            $wbs{$kv}{'flex'} = 0;
            $wbs{$kv}{'preps'} = $wbs{$kv}{'leads'} = 'noconnect';
            $dr = -1;
            $cm = 'noconnect';
            # FIXME: 'fte'
        }
        $dys = $wbs{$kv}{'days'};
        $mjs = $wbs{$kv}{'mjds'};
        print MJDBG "$wbs{$kv}{'nick'} " . " --- " .
            "ok=$ok dr=$dr days=$dys/$mjs --- " .
            "$cm\n" if ($debugmjdupdates);
    }
    close(MJDBG) if ($debugmjdupdates);
    return($mods);
}

#
# Adjust the start or stop date based on successors or predecessors
# for any task that has a $depends on its start/stop time (begin/end).
# if start/needs is depends, look for the largest (latest) end of needs
# if stop/allows is depends, look for the smallest (earliest) begin of allows
# preps and leads are defaulted to milestone (days==0) or noconnect (days<>0)
# we set these ($side) unconstrained until we have the nickname of who set it.
#
# If $est is 0 we only propagate start/stop values.
# When non-zero, we start estimating dates based on 'mjds'.
#
# Return the number of changes, or <0 if we should just stop.
#
sub adjust_dates {
    # start  needs   end  preps
    #  stop  allows begin leads
    my ($dir,$using,$what,$side) = @_;
    my ($mjdo,$kv,$n,$m,$tk,$mods,$odr,@nicks);
    $mods = 0;
    # aha
    if    ($dir eq 'start') { $odr = 'stop';  $mjdo = 0; }
    elsif ($dir eq 'stop')  { $odr = 'start'; $mjdo = 99999; }
    # else { an error that cannot happen... ; }
    for my $kv (keys(%wbs)) {
        next if ($wbs{$kv}{$dir} ne $depends);
        $wbs{$kv}{$side} = 'unconstrained';
        @nicks = split(/,/,$wbs{$kv}{$using});  # needs or allows
        $m = $mjdo;
        for $n (@nicks) {
            $tk = &task_by_nick($n,'adjust_dates');
            next if ($tk eq 'none');    # an error, that cannot happen ...
            next if ($wbs{$tk}{$odr} eq $depends); # no information
            # if later or earlier update
            if (($dir eq 'start' and $wbs{$tk}{$what} > $m) or
                ($dir eq 'stop'  and $wbs{$tk}{$what} < $m)) {
                $m = $wbs{$kv}{$what};
                $wbs{$kv}{$side} = $n;
                $wbs{$kv}{$dir} = $wbs{$tk}{$odr};
                $mods ++;
            }
        }
    }
    return($mods);
}

#
# check that dates are properly ordered
#   &check_dates('start', 'needs', 'begin', 'end', 'stop');
#   &check_dates('stop', 'allows', 'end', 'begin', 'start');
# begin and end are the MJD equivalents of start and stop.
#
sub check_dates {
    #   start needs  begin  end stop
    #   stop allows  end  begin start
    my ($dir,$using,$mine,$yurn,$what) = @_;
    my ($relation,@errs,@nicks);
    # if (not $veryverb) { return; }
    my $na = "needs ";
    $na = "allows" if ($dir eq 'stop');
    if ($veryverb) { open(CD, ">$output.$dir"); }
    else {           open(CD, ">/dev/null"); }
    $errs[2] = $errs[1] = $errs[0] = 0;
    print CD "Checking dates for direction $dir ($mine $yurn)\n\n";
    for my $k (keys(%wbs)) {
        my $kn = $wbs{$k}{'nick'};
        $wbs{$k}{'errors'} = 0 if (not defined($wbs{$k}{'errors'}));
        $wbs{$k}{'errors'} = 0 if ($wbs{$k}{'errors'} eq '');
        next if ($wbs{$k}{'type'} ne 'task');
        @nicks = split(/,/,$wbs{$k}{$using});
        if ($#nicks<0) {
            print CD "No $na  for $kn ... '$wbs{$k}{'errors'}'\n";
        } else {
            print CD "KN $kn with $na $wbs{$k}{$using}\n";
        }
        for my $tn (@nicks) {
            #print CD "  ($tn and $kn)\n";
            my $n = &task_by_nick($tn,'check_dates');
            print CD "  Considering $tn with MJD " . $wbs{$n}{$yurn} . "\n";
            if ($wbs{$k}{'start'} eq $depends or
                $wbs{$k}{'stop'} eq $depends) {
                $errs[2]++;
                $wbs{$k}{'errors'} = -1;
                print CD "   Depends case, moving on\n";
            } elsif ($dir eq 'start' and $wbs{$k}{$mine} < $wbs{$n}{$yurn}) {
                print CD "   ERR: " .
                    $wbs{$k}{'nick'} . " $dir " .
                    $wbs{$k}{$mine} ." < ". $wbs{$n}{$yurn} . " " .
                    $wbs{$n}{'nick'} . " $what\n";
                $errs[1]++;
                $wbs{$k}{'errors'}++;
            } elsif ($dir eq 'stop' and $wbs{$k}{$mine} > $wbs{$n}{$yurn}) {
                print CD "   ERR: " .
                    $wbs{$k}{'nick'} . " $dir " .
                    $wbs{$k}{$mine} ." > ". $wbs{$n}{$yurn} . " " .
                    $wbs{$n}{'nick'} . " $what\n";
                $errs[1]++;
                $wbs{$k}{'errors'}++;
            } else {
                $relation = '>' if ($dir eq 'start');
                $relation = '<' if ($dir eq 'stop');
                print CD "    OK: " .
                    $wbs{$k}{'nick'} . " $dir " .
                    $wbs{$k}{$mine} ." $relation ". $wbs{$n}{$yurn} . " " .
                    $wbs{$n}{'nick'} . " $what\n";
                $errs[0]++;
            }
        }
    }
    close(CD);
    return(@errs);
}

#
# work out the timeline; called from top-level.
#  here we first enforce partial order so that:
#   needs tasks come before
#   allows tasks come after
#  update begin/end MJDs
#  compute mjds/flex allowances for resolved tasks
#  update start/stop for task with $depends from needs/allows
# once we have run out of updates, then we allow estimation of start/stop
#  which is tracked through the $est variable: 0 means not allowed.
#
sub work_out_timeline {
    my ($mods,$pass,$est) = (1,0,0);
    my ($maxpass,$maxest, @ers) = ($maxtimelinepasses,$maxtimelineestimates);
    while ($mods) {
        print "# Pass $pass/$maxpass; est $est/$maxest:\t" if ($veryverb);
        $mods = &update_mjd_data($est);
        print "update: $mods " if ($veryverb);
        $mods += &adjust_dates('start','needs','end','preps');
        print "preps:  $mods " if ($veryverb);
        $mods += &adjust_dates('stop','allows','begin','leads');
        print "leads:  $mods\n" if ($veryverb);
        $pass ++;
        if ($mods == 0 and $est < $maxest) { $mods = -1; $est++; }
        last if ($pass >= $maxpass or $mods == 0);
    }
    $mods = &update_mjd_data($est);
    print "# Final $mods; est $est; checking dates now...\n" if ($veryverb);
    @ers = &check_dates('start', 'needs', 'begin', 'end', 'stop');
    print "# start checks found $ers[0] ok and $ers[1] bad $ers[2] depends\n";
    @ers = &check_dates('stop', 'allows', 'end', 'begin', 'start');
    print "# stop  checks found $ers[0] ok and $ers[1] bad $ers[2] depends\n";
    print "# Finished checks\n" if ($veryverb);
}

#
# helper function called from top-level that adjusts the 'shape' (and
# possibly other attributes) based on node contents.  Currently it
# changes the default ellipse to rectangle for tasks with timeline errors
# or egg for tasks with start or stop still marked $depends.
#
sub assign_node_attributes {
    for my $k (keys(%wbs)) {
        # my $kn = $wbs{$k}{'nick'};
        next if ($wbs{$k}{'type'} ne 'task');
        if ($wbs{$k}{'errors'} eq '') { $wbs{$k}{'shape'} = 'hexagon'; }
        elsif ($wbs{$k}{'errors'} > 0) { $wbs{$k}{'shape'} = 'rectangle'; }
        elsif ($wbs{$k}{'errors'} < 0) { $wbs{$k}{'shape'} = 'egg'; }
        else { $wbs{$k}{'shape'} = 'ellipse'; }
    }
}

#
# helper functions to complete needs<->allows references
# the logic is the actually the same, whether it is 
#   allows (implies mention in needs) or
#   needs (implies mention in allows)
#
sub make_needs_or_allows {
    my ($allows,$needs,$k,$t,$n,$me,@a) = @_;
    return if ($skipneedsallows > 0);
    for $k (keys(%wbs)) {
        if (!defined($wbs{$k}{'nick'})) {
            print "$k has no nick\n";
            next;
        }
        $me = $wbs{$k}{'nick'};
        if ($me eq 'none' or $me eq '') {
            print "Nick for $k is '$me;\n";
            next;
        }

        @a = split(/,/,$wbs{$k}{$allows});
        for $n (@a) {
            if ($n eq '') {
                print "Empty '$n' as $allows of $k\n";
                next;
            }
            $t = &task_by_nick($n,'make_needs_or_allows');
            if ($t eq 'none') {
                print "Nick for '$n' ($allows) is '$t'\n";
                next;
            }
            if (defined($wbs{$t}{$needs}) and
                $wbs{$t}{$needs} ne '') {
                $_ = $wbs{$t}{$needs};
            } else {
                $wbs{$t}{$needs} = $me;
                next;
            }
            next if (m/,$me/ or m/$me,/ or m/,$me,/ or m/^$me$/);
            $wbs{$t}{$needs} .= ',' . $me;
        }
    }
}

# called from top-level to populate 'needs' entries based on 'allows'
sub make_needs_from_allows {
    print "make_needs_from_allows\n" if ($verb);
    &make_needs_or_allows('allows','needs');
}
# called from top-level to populate 'allows' entries based on 'needs'
sub make_allows_from_needs {
    print "make_allows_from_needs\n" if ($verb);
    &make_needs_or_allows('needs','allows');
}

#
# this is a library module, so it needs to return TRUE.
#
1;
#
# eof
#
