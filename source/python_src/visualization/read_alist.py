# Functions to support parsing an alist file
import numpy as np


# alist files have many columns, pick out the useful information
# for a given baseline, return amplitudes and time tags
def get_amp_time(filename, baseline):


    f = open(filename, 'r')
    lines = f.readlines()
    f.close()

    source = []
    scan = []
    root = []
    amp = []
    t = []
    base = []
    pol = []
    
    for line in lines:

        # skip header lines
        if line[0]=='*':
            continue
        cols = line.rsplit('\n')[0].split() # remove the trailing EOL and split on whitespace

        # build lists of the useful columns
        root.append(cols[1])

        # TIME*TAG column is in fractional DOY format?
        timetag = cols[11].split('-')
        t.append(int(timetag[0])+float('0.'+timetag[1]))

        source.append(cols[13][0:5]) # only grab the first five characters of the source name
        base.append(cols[14])
        pol.append(cols[17])
        amp.append(float(cols[19]))

    # find the indices of the rows with data from the baseline we want
    idx = np.where(np.array(base, copy=False) == baseline)[0]

    # convert lists to arrays for indexing
    amplitudes = np.array(amp)[idx]
    times = np.array(t)[idx]

    # construct the filename for the jpg file (kludge)
    fname = []
    for ii in range(len(idx)):
        fname.append(base[idx[ii]] + '_' + source[idx[ii]] + '_' + pol[idx[ii]])
        
    return fname, amplitudes, times
