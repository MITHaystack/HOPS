#
# some additional configuration items
#

#
# preloads of color choices for MHO
# grep -h ' => .*[0-9], ' fruit/* | sort | uniq | cut -c3-
#
our %style_preload = (
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
