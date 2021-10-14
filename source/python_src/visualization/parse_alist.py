# Functions to support parsing an alist file
# eventually this will be replaced with a new format??
import numpy as np

#from collections import defaultdict

# should include some sanity checks and error handling

# methods to get indices of alist records to plot, given lists of selections
# use python set methods? intersection is simple

# method: given dictionaries of selected parameters, return indices of alist records to plot

# This module makes many assumptions about alist data format! How reliable is this format?
# Need to store units for each field

# list of dictionary fields for each record in the alist file
alist_fields = ['source', # phonebook name for the source observed
                'scan', # scan name, eg 105-0347 or 210-1800a (DOY-HHMM[a,b,c])
                'baseline', # two-letter baseline for each record
                'stations', # two-element list of single-letter station identifiers
                'scan_DOY', # scan day of year
                'scan_time', # scan time in fractional DOY format (DOY.1234)
                'scan_length', # scan length in seconds (different from DUR column)
                'pols', # two-letter description of polarization combination for this baseline (eg XX, RR, XY, YR, etc)
                'qcode', # scan quality code (0-9, A-H)
                'amplitude', # amplitude in units of 1e-4
                'snr',
                'sbd', # single-band delay
                'mbd', # multi-band delay
                'delay_rate']

                



class ParseAlist:

    def __init__(self, afile):


        self.records = dict.fromkeys(alist_fields)
        #self.records = defaultdict(list)

        # if we initialize all the dict values to a list with fromkeys(), it will be the *same* list
        # so initialize the values in a loop
        for field in alist_fields:
            self.records[field] = []
        
        self.unique_stations = [] # stations are stored as pairs, store the list of unique stations separately
        
        self.start_day = 0 # for plotting?

        
        f = open(afile, 'r')
        lines = f.readlines()
        f.close()

        for line in lines:

            # skip header lines
            if line[0]=='*':
                continue
            cols = line.rsplit('\n')[0].split() # remove the trailing EOL and split on whitespace

            # build lists of the useful columns
            #root.append(cols[1])

            self.records['scan_length'].append(cols[5])

            self.records['scan'].append(cols[8])
            
            # TIME*TAG column is in DOY-HHMMSS format
            # reformat into fractional DOY
            timetag = cols[11].split('-')
            fractional_day = float(timetag[1][0:2])*3600 + float(timetag[1][2:4])*60 + float(timetag[1][4:6])
            self.records['scan_time'].append(int(timetag[0])+fractional_day/86400.)
            
            self.records['source'].append(cols[13])
            self.records['baseline'].append(cols[14])
            self.records['qcode'].append(cols[15])
            self.records['pols'].append(cols[17])
            self.records['amplitude'].append(float(cols[19]))
            self.records['snr'].append(float(cols[20]))
            self.records['sbd'].append(float(cols[24]))
            self.records['mbd'].append(float(cols[25]))
            self.records['delay_rate'].append(float(cols[27]))


        # sanitize qcodes - if there's a letter, just save the letter
        for ii in range(len(self.records['qcode'])):
            if len(self.records['qcode'][ii])>1:
                self.records['qcode'][ii]=self.records['qcode'][ii][1]

        # work out the number of each qcode
        self.unique_qcodes, self.qcode_counts = np.unique(self.records['qcode'], return_counts=True)

        # work out the list of stations present in the data
        self.records['stations'] = [[xx[0],xx[1]] for xx in self.records['baseline']]
        self.unique_stations = np.unique(np.concatenate(self.records['stations']).flat)





    # method to return list of record indices meeting criteria in data_selection_dict
    def get_record_indices(self, data_selection_dict):

        # baselines, stations, sources, polarizations, qcodes, snrrange

        baseline_idx = [i for i,x in enumerate(self.records['baseline']) if x in data_selection_dict['baselines']]

        # stations have to be handled backwards...
        station_idx = []
        for station in data_selection_dict['stations']:
            station_idx.extend([i for i,x in enumerate(self.records['stations']) if station in x])

        #print(np.array(self.baselines)[baseline_idx])
        #print(np.array(self.stations)[station_idx])
        
        return list(set(baseline_idx) & set(station_idx))


