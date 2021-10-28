# Functions to support parsing an alist file
# eventually this will be replaced with a new format??
import numpy as np

#from collections import defaultdict

# should include some sanity checks and error handling



# list of dictionary fields for each record in the alist file
alist_fields = ['index', # unique field to keep track of record
                'experiment', # four-digit experiment label?
                'source', # phonebook name for the source observed
                'scan', # scan name, eg 105-0347 or 210-1800a (DOY-HHMM[a,b,c])
                'baseline', # two-letter baseline for each record
                'stations', # two-element list of single-letter station identifiers
                'scan_DOY', # scan day of year
                #'scan_year',
                #'proc_date',
                'frequency_band', # band and number of channels?
                'scan_timetag', # time tag of scan (unparsed string)
                'scan_time', # scan time in fractional DOY format (DOY.1234)
                'scan_length', # scan length in seconds (different from DUR column)
                'pols', # two-letter description of polarization combination for this baseline (eg XX, RR, XY, YR, etc)
                'qcode', # scan quality code (0-9, A-H)
                'amplitude', # amplitude in units of 1e-4
                'snr',
                'phase',
                'sbd', # single-band delay
                'mbd', # multi-band delay
                'delay_rate',
                'ambiguity'] # is this always the same for every record?

                
# list of plot fields for drop-down menus
plot_fields = ['scan_time', 'amplitude', 'snr', 'phase', 'sbd', 'mbd', 'delay_rate','ambiguity']


# list of plot labels and units
plot_labels = {'scan_time':{'label':'Scan time','unit':'(DOY)'},
               'amplitude':{'label':'Amplitude','unit':'(x10^4)'},
               'snr':{'label':'SNR','unit':''},
               'phase':{'label':'Phase','unit':'(deg)'},
               'sbd':{'label':'Single-band delay','unit':'(usec)'},
               'mbd':{'label':'Multi-band delay','unit':'(usec)'},
               'delay_rate':{'label':'Delay rate','unit':'ps/s'},
               'ambiguity':{'label':'Ambiguity','unit':'usec'}}

# kludge to convert pretty names to the names of fields in the alist records
plot_label_convert = {'Source':'source',
                      'Station':'stations',
                      'QCode':'qcode',
                      'Polarization':'pols',
                      'Baseline':'baseline',
                      'Phase':'phase',
                      'Scan time':'scan_time',
                      'Amplitude':'amplitude',
                      'SNR':'snr',
                      'Single-band delay':'sbd',
                      'Multi-band delay':'mbd',
                      'Delay rate':'delay_rate',
                      'Ambiguity':'ambiguity'}


# helper class that does not always work - some alist files have a varying number of header lines
class AfileData:

    def __init__(self, afile):
        f = open(afile, 'r')
        self.lines = f.readlines()
        f.close()
        
        self.labels = self.lines[2].rsplit('\n')[0].split()
        self.units = self.lines[3].rsplit('\n')[0].split()
        self.d1 = self.lines[4].rsplit('\n')[0].split()
        



# not all alist files are formatted the same
# some have an extra header line (of column numbers?)
# there can be 40 or 46 columns
# 40 column files have a truncated units line (only 23)
# columns in both formats are the same up to col 27 (DRATE)

class ParseAlist:

    def __init__(self):

        self.records = dict.fromkeys(alist_fields)
        #self.records = defaultdict(list)

        # if we initialize all the dict values to a list with fromkeys(), it will be the *same* list
        # so initialize the values in a loop
        for field in alist_fields:
            self.records[field] = []
        
        self.unique_stations = [] # stations are stored as pairs, store the list of unique stations separately
        
        self.start_day = 0 # for plotting?

        self.counter=0

        
    def load_alist_data(self, afile):

        f = open(afile, 'r')
        lines = f.readlines()
        f.close()
        
        header_counter = 0 # to keep track of the format for saving a subset of records
        for line in lines:

            # skip header lines
            if line[0]=='*':
                header_counter += 1
                continue
            cols = line.rsplit('\n')[0].split() # remove the trailing EOL and split on whitespace

            # build lists of the useful columns
            #root.append(cols[1])

            self.records['index'].append(self.counter) # this is useful for the event picking in aedit
            self.records['scan_length'].append(cols[5])
            self.records['experiment'].append(cols[7])
            self.records['scan'].append(cols[8])
            
            # TIME*TAG column is in DOY-HHMMSS format
            # reformat into fractional DOY
            timetag = cols[11].split('-')
            self.records['scan_timetag'].append(cols[11])
            fractional_day = float(timetag[1][0:2])*3600 + float(timetag[1][2:4])*60 + float(timetag[1][4:6])
            self.records['scan_time'].append(int(timetag[0])+fractional_day/86400.)
            
            self.records['source'].append(cols[13])
            self.records['baseline'].append(cols[14])
            
            # sanitize qcodes - if there are two characters and the second is a letter, just save the letter
            qc = cols[15]
            if len(qc)>1 and qc[1].isalpha():
                self.records['qcode'].append(qc[1])
            else:
                self.records['qcode'].append(qc)

            # frequency band is often B32 or X32 or similar...if the first character is a letter, use that
            freq = cols[16]
            if freq[0].isalpha():
                self.records['frequency_band'].append(freq[0])
            else:
                self.records['frequency_band'].append(freq)
                
            self.records['pols'].append(cols[17])
            self.records['amplitude'].append(float(cols[19]))
            self.records['snr'].append(float(cols[20]))
            self.records['phase'].append(float(cols[21]))
            self.records['sbd'].append(float(cols[24]))
            self.records['mbd'].append(float(cols[25]))
            self.records['ambiguity'].append(float(cols[26]))
            self.records['delay_rate'].append(float(cols[27]))

            #print(cols[8], cols[14], cols[17], cols[26])
            
            self.counter+=1

        
        #for ii in range(len(self.records['qcode'])):
        #    if len(self.records['qcode'][ii])>1:
        #        self.records['qcode'][ii]=self.records['qcode'][ii][1]

        # work out the number of each qcode
        #self.unique_qcodes, self.qcode_counts = np.unique(self.records['qcode'], return_counts=True)

        # work out the list of stations present in the data
        self.records['stations'] = [[xx[0],xx[1]] for xx in self.records['baseline']]
        self.unique_stations = np.unique(np.concatenate(self.records['stations']).flat)

        # keep an array of flag state for each record
        # this will reset the flags, if you are merging new data!
        self.record_flags = np.zeros(len(self.records['snr']))

        # keep an array of colors defined by the SNR of each record
        self.record_color = ['black']*len(self.records['snr'])


        

    # method to return list of record indices meeting criteria in data_selection_dict
    def get_record_indices(self, data_selection_dict, scan_selection_dict):

        # baselines, stations, sources, polarizations, qcodes, snrrange

        #print(data_selection_dict)

        baseline_idx = [i for i,x in enumerate(self.records['baseline']) if x in data_selection_dict['baselines']]
        source_idx = [i for i,x in enumerate(self.records['source']) if x in data_selection_dict['sources']]
        qcode_idx = [i for i,x in enumerate(self.records['qcode']) if x in data_selection_dict['qcodes']]
        pols_idx = [i for i,x in enumerate(self.records['pols']) if x in data_selection_dict['pols']]

        scan_idx = [i for i,x in enumerate(self.records['scan']) if x in scan_selection_dict['scans']]

        snrmin_idx = [i for i,x in enumerate(self.records['snr']) if x>=data_selection_dict['snrrange'][0]]
        snrmax_idx = [i for i,x in enumerate(self.records['snr']) if x<=data_selection_dict['snrrange'][1]]
        
        
        # for now flag records if an un-selected station is present
        # this means if you select only one station, all records will be flagged...
        station_idx = list(range(len(self.records['stations'])))
        for ii in range(len(station_idx)):
            for station in data_selection_dict['not_stations']:
                if station in self.records['stations'][ii]:
                    station_idx.remove(ii)
                    break
                else:
                    pass


        # old station selection code
        #station_idx = []
        #for station in data_selection_dict['stations']:
        #    station_idx.extend([i for i,x in enumerate(self.records['stations']) if station in x])

        selected_idx = list(set(baseline_idx) & set(station_idx) & set(source_idx) & set(qcode_idx) & set(pols_idx) & \
                            set(scan_idx) & set(snrmin_idx) & set(snrmax_idx))

        # set only the selected records to flag=0
        self.record_flags.fill(1)
        self.record_flags[selected_idx] = 0
        
        return selected_idx


    # method to define a list of colors for plotting
    def set_record_colors(self, plot_format_dict):

        if plot_format_dict['point_color']['color_by_SNR']:

            good_idx = np.where(np.array(self.records['snr'])>=plot_format_dict['point_color']['good_threshold'])[0]
            suspect_idx = np.where(np.array(self.records['snr'])<plot_format_dict['point_color']['suspect_threshold'])[0]
            bad_idx = np.where(np.array(self.records['snr'])<=plot_format_dict['point_color']['bad_threshold'])[0]
        
            for jj in suspect_idx:
                self.record_color[jj] = 'darkorange'
            for ii in bad_idx:
                self.record_color[ii] = 'red'
            for kk in good_idx:
                self.record_color[kk] = 'royalblue'

            return
            
        else:
            # reset all the colors
            self.record_color = ['black']*len(self.records['snr'])
            return
            
