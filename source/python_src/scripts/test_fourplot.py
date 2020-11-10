
import sys, os
# temporary command to add modules to path
sys.path.insert(1,'../io')
sys.path.insert(1,'../visualization')

from parse_pdd_file import parse_PDD

from fourfit_plot import fourfit_plot

plot_dict = parse_PDD('/home/dhoak/Desktop/haystack/HOPS/build/data/ff_testdata/pdd2843/321-1701_0552+398-AI-S-RR.oifhak')

fourfit_plot(plot_dict, 'test_fringe_plot.pdf')

