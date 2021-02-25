#
# some additional configuration items
#

#
# preloads of color choices for MHO
# grep -h ' => .*[0-9], ' fruit/* | sort | uniq | cut -c3-
#
our %style_preload = (
 'Everything Else%%Project Time Margin' => 6,
 'General Project Activities%%Meetings and Other Activities' => 2,
 'General Project Activities%%Milestonestones' => 3,
 'HOPS4 Documentation%%Coverage and Testing' => 5,
 'HOPS4 Documentation%%Requirements' => 0,
 'HOPS4 Documentation%%Software Development Plan' => 1,
 'HOPS4 Documentation%%Specifications' => 4,
);

#
# dot graph prefixes
# $output . '-'. 'plotname' => 'dot graph preamble items',
# 
our %preamble_stuff = (
);

#
# this is a library module, so it needs to return TRUE.
#
1;
#
# eof
#
