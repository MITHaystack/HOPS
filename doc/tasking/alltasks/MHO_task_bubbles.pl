#
# some additional configuration items
#

#
# preloads of color choices for MHO -- this is how you can stabilize the
# (random) color choices so that you start seeing the same thing  with the
# same color.  The key here is 'full name of domain%%full name of thing'.
#
# grep -h ' => .*[0-9], ' fruit/* | sort | uniq | cut -c3-
#
our %style_preload = (
  'General Project Activities%%Milestones for the project' => 18,
  'General Project Activities%%Meetings and Other Activities' => 19,
  'HOPS4 Documentation%%Project Development Plan' => 20,
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
