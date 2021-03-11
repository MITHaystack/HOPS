#!/usr/bin/perl
#
use warnings;           # turns on optional warnings
use diagnostics;        # and makes them not terse
use strict;             # makes unsafe constructs illegal
#
# Support script for computing timeline; with hops_time:
# $ HOPS_JULIAN_EPOCH=UTC hops_time -q I 56763.0 |cut -dT -f1
# $ HOPS_JULIAN_EPOCH=UTC hops_time -q M 2014-04-16 |cut -d. -f1|sed 's/$/.0/'
#
# Calendar dates are of the form YYYY-MM-DD <=> MJD (5 digits)
# 1995-10-10                        50000
# 2011-08-15 when contract awarded  55788
# 2012-02-29 (leap day)             55986
# 2016-02-29 (leap day)             57447
# 2016-08-15 six years afterwards   57615
# 2023-02-25                        60000
#
# With Python3 datetime module gets you a unix timestamp:
# datetime.datetime.fromisoformat(
#   '2021-01-01T00:00:00').replace(tzinfo=datetime.timezone.utc).timestamp()
# = 1609459200.0 unix clock
#
# def MJD(isotime):
#    zulu = 'T00:00:00'
#    tsn = datetime.datetime.fromisoformat(isotime + zulu).replace(
#       tzinfo=datetime.timezone.utc).timestamp()
#    ts0 = datetime.datetime.fromisoformat('1995-10-10' + zulu).replace(
#       tzinfo=datetime.timezone.utc).timestamp()
#    return (50000 + int(tsn - ts0)//86400)
#
# note that all of the above is calendar time (i.e. ignoring leap seconds)

our %wbs;
our ($date_pat,$mjd_pat);
our ($verb,$veryverb);

#my ($gBegin,$gEnd) = (55788,57249);     # 2011-08-15 - 2015-08-15
our ($gBegin,$gEnd);

# skip generation of needs / allows
our $skipneedsallows;
# allow debugging of the math
our ($debugmjdupdates,$output);

#
# This routine computes begin/end from start/stop
# and assigns the time required and margin members.
# Undefined start/stop times are canonicalized to 'depends'
# for the following adjustment phases.
#
sub update_mjd_data {
    my ($ok,$gap,$mods,$m,$dr,$dys,$mjs,$cm) = (0,0,0);
    open(MJDBG,">$output" . '-mjdupdate.dbg') if ($debugmjdupdates);
    for my $kv (keys(%wbs)) {
        $ok = 0;
        if ($wbs{$kv}{'start'} =~ m/$date_pat/) {
            $m = &date_to_mjd($wbs{$kv}{'start'});
            $mods++ if ($m ne $wbs{$kv}{'begin'});
            $wbs{$kv}{'begin'} = $m;
            $ok++;
        } else {
            $wbs{$kv}{'start'} = 'depends';
            $wbs{$kv}{'begin'} = $gBegin;
            $ok = 0;
        }
        if ($wbs{$kv}{'stop'} =~ m/$date_pat/) {
            $m = &date_to_mjd($wbs{$kv}{'stop'});
            $mods++ if ($m ne $wbs{$kv}{'end'});
            $wbs{$kv}{'end'} = $m;
            $ok++;
        } else {
            $wbs{$kv}{'stop'} = 'depends';
            $wbs{$kv}{'end'} = $gEnd;
            $ok = 0;
        }
        if (($ok eq 2) and ($wbs{$kv}{'days'} > 0)) {
            $gap = $wbs{$kv}{'end'} - $wbs{$kv}{'begin'};
            $wbs{$kv}{'mjds'} = $wbs{$kv}{'days'} * $wbs{$kv}{'derate'};
            $wbs{$kv}{'flex'} = $gap - $wbs{$kv}{'mjds'};
            $dr = $wbs{$kv}{'derate'};
            $cm = 'ok';
        } elsif ($wbs{$kv}{'days'} eq 0) {    # a milestone
            $wbs{$kv}{'mjds'} = 0;
            $wbs{$kv}{'flex'} = 0;
            $wbs{$kv}{'preps'} = $wbs{$kv}{'leads'} = 'milestone';
            $dr = 0;
            $cm = 'milestone';
        } else {
            $wbs{$kv}{'mjds'} = 0;
            $wbs{$kv}{'flex'} = 0;
            $wbs{$kv}{'preps'} = $wbs{$kv}{'leads'} = 'noconnect';
            $dr = -1;
            $cm = 'noconnect';
        }
        $dys = $wbs{$kv}{'days'};
        $mjs = $wbs{$kv}{'mjds'};
        print MJDBG
            "$kv ok=$ok dr=$dr days=$dys/$mjs $cm\n" if ($debugmjdupdates);
    }
    close(MJDBG) if ($debugmjdupdates);
    return($mods);
}

#
# Adjust the start or stop date based on the predecessors or successors
# for any task that has a 'depends' start/stop time.
# if start/needs, look for the largest begin
# if stop/allows, look for the smallest end
#
sub adjust_dates {
    my ($dir,$using,$what,$side,$mjdo,$kv,$n,$m,$tk,$mods,@nicks) = @_;
    $mods = 0;
    for my $kv (keys(%wbs)) {
        next if ($wbs{$kv}{$dir} ne 'depends');
        $wbs{$kv}{$side} = 'unconstrained';
        @nicks = split(/,/,$wbs{$kv}{$using});
        $m = $mjdo;
        for $n (@nicks) {
            $tk = &task_by_nick($n,'adjust_dates');
            next if ($tk eq 'none');    # an error, actually
            next if ($wbs{$tk}{$dir} eq 'depends');
            if (($dir eq 'start' and $wbs{$tk}{$what} > $m) or
                ($dir eq 'stop'  and $wbs{$tk}{$what} < $m)) {
                $m = $wbs{$kv}{$what};
                $wbs{$kv}{$side} = $n;
                $wbs{$kv}{$dir} = $wbs{$tk}{$dir};
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
#
sub check_dates {
    #   start needs  begin  end stop
    #   stop allows  end  begin start
    my ($dir,$using,$mine,$yurn,$what,@errs,@nicks) = @_;
    if (not $veryverb) { return; }
    open(CD, ">$output.$dir");
    $errs[1] = $errs[0] = 0;
    print CD "Checking dates for direction $dir\n\n";
    for my $k (keys(%wbs)) {
        next if ($wbs{$k}{'type'} ne 'task');
        @nicks = split(/,/,$wbs{$k}{$using});
        for my $tn (@nicks) {
            my $n = &task_by_nick($tn,'check_dates');
            print CD "Considering $n with MJD " . $wbs{$n}{$yurn} . "\n";
            if ($dir eq 'start' and $wbs{$k}{$mine} < $wbs{$n}{$yurn}) {
                print CD " OK: " .
                    $wbs{$k}{'nick'} . " $dir " .
                    $wbs{$k}{$mine} ." < ". $wbs{$n}{$yurn} . " " .
                    $wbs{$n}{'nick'} . " $what\n";
                $errs[0]++;
            } elsif ($dir eq 'stop' and $wbs{$k}{$mine} > $wbs{$n}{$yurn}) {
                print CD " OK: " .
                    $wbs{$k}{'nick'} . " $dir " .
                    $wbs{$k}{$mine} ." > ". $wbs{$n}{$yurn} . " " .
                    $wbs{$n}{'nick'} . " $what\n";
                $errs[0]++;
            } else {
                print CD "ERR: " .
                    $wbs{$k}{'nick'} . " $dir " .
                    $wbs{$k}{$mine} ." ? ". $wbs{$n}{$yurn} . " " .
                    $wbs{$n}{'nick'} . " $what\n";
                $errs[1]++;
            }
        }
    }
    close(CD);
    return(@errs);
}

#
# work out the timeline
#  enforce partial order so that:
#   needs tasks come before
#   allows tasks come after
#  update begin/end MJDs
#  compute mjds/flex allowances
#  update start/stop for task
#
sub work_out_timeline {
    my ($mods,$pass) = (1,0);
    my (@ers);
    while ($mods) {
        print "# Pass $pass:\t" if ($veryverb);
        $mods = &update_mjd_data();
        print "update: $mods " if ($veryverb);
        $mods += &adjust_dates('start','needs','begin','preps',0);
        print "preps:  $mods " if ($veryverb);
        $mods += &adjust_dates('stop','allows','end','leads',99999);
        print "leads:  $mods\n" if ($veryverb);
        $pass ++;
        last if ($pass > 19 or $mods eq 0);
    }
    $mods = &update_mjd_data();
    print "# Final $mods; checking dates now...\n" if ($veryverb);
    @ers = &check_dates('start', 'needs', 'begin', 'end', 'stop');
    print "# start checks found $ers[0] ok and $ers[1] bad\n";
    @ers = &check_dates('stop', 'allows', 'end', 'begin', 'start');
    print "# stop  checks found $ers[0] ok and $ers[1] bad\n";
    print "# Finished checks\n" if ($veryverb);
}

#
# helper function that adjusts the 'shape' (and possibly other attributes)
# based on node contents.
#
sub assign_node_attributes {
    print "# Not yet implemented\n" if ($veryverb);
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
            # FIXME: why?
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
sub make_needs_from_allows {
    print "make_needs_from_allows\n" if ($verb);
    &make_needs_or_allows('allows','needs');
}
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
