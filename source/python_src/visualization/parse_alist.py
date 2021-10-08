# Functions to support parsing an alist file
# eventually this will be replaced with a new format??
import numpy as np



# should include some sanity checks and error handling


class ParseAlist:

    def __init__(self, afile):

        self.sources = []
        self.scans = []
        self.amplitudes = []
        self.times = []
        self.baselines = []
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
        codes, counts = np.unique(self.qcodes, return_counts=True)

            
    # find the indices of the rows with data from the baseline we want
    #idx = np.where(np.array(base, copy=False) == baseline)[0]

    # convert lists to arrays for indexing
    #amplitudes = np.array(amp)[idx]
    #times = np.array(t)[idx]

