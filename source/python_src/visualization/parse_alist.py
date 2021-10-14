# Functions to support parsing an alist file
# eventually this will be replaced with a new format??
import numpy as np



# should include some sanity checks and error handling

# methods to get indices of alist records to plot, given lists of selections
# use python set methods? intersection is simple

# method: given dictionaries of selected parameters, return indices of alist records to plot



class ParseAlist:

    def __init__(self, afile):

        self.sources = []
        self.scans = []
        self.amplitudes = []
        self.times = []
        self.baselines = []
        self.stations = []
        self.unique_stations = [] # stations are stored as pairs, so get the unique list separately
        self.pols = []
        self.qcodes = []
        self.snr = []
        self.sbd = []
        self.mbd = []
        self.delay_rate = []
        self.length = []
        
        
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

            self.scans.append(cols[8])
            
            # TIME*TAG column is in fractional DOY format?
            timetag = cols[11].split('-')
            self.times.append(int(timetag[0])+float('0.'+timetag[1]))
            
            #self.sources.append(cols[13][0:5]) # only grab the first five characters of the source name
            self.sources.append(cols[13])
            self.baselines.append(cols[14])
            self.qcodes.append(cols[15])
            self.pols.append(cols[17])
            self.amplitudes.append(float(cols[19]))
            self.snr.append(float(cols[20]))
            self.sbd.append(float(cols[24]))
            self.mbd.append(float(cols[25]))
            self.delay_rate.append(float(cols[27]))
            # snr, sbd, mdb, delay_rate

        
        # sanitize qcodes - if there's a letter, just save the letter
        for ii in range(len(self.qcodes)):
            if len(self.qcodes[ii])>1:
                self.qcodes[ii]=self.qcodes[ii][1]
                
        self.unique_qcodes, self.qcode_counts = np.unique(self.qcodes, return_counts=True)

        self.stations = [[xx[0],xx[1]] for xx in self.baselines]

        self.unique_stations = np.unique(np.concatenate(self.stations).flat)







    # method to return list of record indices meeting criteria in data_selection_dict
    def get_record_indices(self, data_selection_dict):

        # baselines, stations, sources, polarizations, qcodes, snrrange

        baseline_idx = [i for i,x in enumerate(self.baselines) if x in data_selection_dict['baselines']]

        # stations have to be handled backwards...
        station_idx = []
        for station in data_selection_dict['stations']:
            station_idx.extend([i for i,x in enumerate(self.stations) if station in x])

        #print(np.array(self.baselines)[baseline_idx])
        #print(np.array(self.stations)[station_idx])
        
        return list(set(baseline_idx) & set(station_idx))


        
    # find the indices of the rows with data from the baseline we want
    #idx = np.where(np.array(base, copy=False) == baseline)[0]

    # convert lists to arrays for indexing
    #amplitudes = np.array(amp)[idx]
    #times = np.array(t)[idx]

