
import sys

from PyQt5.QtCore import QSize, Qt, QLine, QPoint
from PyQt5.QtWidgets import QApplication, QWidget, QPushButton, QMainWindow, QCheckBox, QFrame, QSizePolicy
from PyQt5.QtWidgets import QVBoxLayout, QHBoxLayout, QGridLayout, QTabWidget, QLabel, QLineEdit
#from PyQt5.QtGui import QPalette, QColor, QPainter, QWindow

from parse_alist import ParseAlist

import matplotlib.pyplot as plt
import alist_plot as alist_plt

import numpy as np

# Main Window: load alist file(s)
# Data Window: tabs

# selection tab selects baselines, sources, etc - summary button?
# plot tab has plotting parameters, plot button -> picking, rerun fourfit with new parameters, cycle through plots
# calc tab does closure -> list available triangles? (from stations from each scan?)

# scans go on a different tab - there are usually a lot of them

# parse alist and get baselines, sources, stations, scans
# build new window with checkboxes for all of these (unique)
# option to select all, remove autocorr, etc
# also have snrmin and snrmax options
# length, frequency

# how is frequency reported in the alist? does 'B32' mean band B, 32 channels?
#

# clever ways to get indices of alist records to plot with pythons sets, intersections



# different tab: select plotting options (axes, limits)

# option to write new afile

# organize plots by baseline? in a list, with a key connection to scroll through them?

# picking: left click, open new window with fourfit plot
# right click, rerun fourfit, open control file and adjust parameters?

# option to show fourfit plot or rerun fourfit for this scan/baseline?

# build plotting options on case
# labels, units, range depending on data type


# this class is an example of a two-box grid
class SelectionParamBoxGrid(QWidget):

    def __init__(self, alist_data, orient='col'):
        super().__init__()

        self.snrmin = np.min(alist_data.records['snr'])
        self.snrmax = np.max(alist_data.records['snr'])
        
        # Create a text entry box
        snrmin_box_label = QLabel('Min SNR')
        self.snrmin_box = QLineEdit()
        self.snrmin_box.setText(str(self.snrmin))
        self.snrmin_box.setFixedWidth(100)

        snrmax_box_label = QLabel('Max SNR')
        self.snrmax_box = QLineEdit()
        self.snrmax_box.setText(str(self.snrmax))
        self.snrmax_box.setFixedWidth(100)

        # Set up the parameters area
        self.param_label = QLabel('Parameters')
        self.params_hbox = QHBoxLayout()
        self.params_hbox.addWidget(self.param_label)
        self.params_hbox.addStretch(1)

        self.separator = QFrame()
        self.separator.setFrameShape(QFrame.HLine)
        self.separator.setLineWidth(1)

        self.textbox_gridv = QVBoxLayout()

        self.textbox_gridh1 = QHBoxLayout()
        self.textbox_gridh1.addWidget(snrmin_box_label)
        self.textbox_gridh1.addWidget(self.snrmin_box)
        self.textbox_gridh1.addStretch(1)
        
        self.textbox_gridh2 = QHBoxLayout()
        self.textbox_gridh2.addWidget(snrmax_box_label)
        self.textbox_gridh2.addWidget(self.snrmax_box)
        self.textbox_gridh2.addStretch(1)

        self.textbox_gridv.addLayout(self.textbox_gridh1)
        self.textbox_gridv.addLayout(self.textbox_gridh2)
        self.textbox_gridv.addStretch(1)

    def collectParamVals(self):

        snrrange = [self.snrmin_box.text(), self.snrmax_box.text()]






# For qcode, we want to list all of them, but grey out the ones that aren't present in the data
# also, provide the number of records with each qcode
class QCodeCheckboxGrid(QWidget):

    def __init__(self, qcode_list):
        super().__init__()

        self.existing_qcodes, self.qcode_counts = np.unique(qcode_list,return_counts=True)

        # not sure if A, N, ? are always used, or are redundant?
        self.checkbox_items = ['0','1','2','3','4','5','6','7','8','9','A','B','C','D','E','F','G','H','N','?']
        self.num_boxes = len(self.checkbox_items)
        self.checkboxes = []

        self.checkbox_grid = QGridLayout()

        positions = [(i, j) for i in range(2) for j in range(10)]
        for position, box_name in zip(positions, self.checkbox_items):
            if box_name in self.existing_qcodes:
                idx = self.existing_qcodes.tolist().index(box_name)
                chkbox = QCheckBox(box_name+' ('+str(self.qcode_counts[idx])+')', self)
                chkbox.setCheckable(True)
                self.checkbox_grid.addWidget(chkbox, *position)
                self.checkboxes.append(chkbox)
            else:
                chkbox = QCheckBox(box_name+' (0)', self)
                chkbox.setCheckable(False)
                chkbox.setDisabled(True)
                self.checkbox_grid.addWidget(chkbox, *position)
                self.checkboxes.append(chkbox)
                
        
        self.grid_label = QLabel('Quality Codes ('+str(len(qcode_list))+' total records)')
        self.label_hbox = QHBoxLayout()
        self.label_hbox.addWidget(self.grid_label)
        self.label_hbox.addStretch(1)

        self.separator = QFrame()
        self.separator.setFrameShape(QFrame.HLine)
        self.separator.setLineWidth(1)

        self.selectAllButton = QPushButton("Select All")
        self.selectAllButton.clicked.connect(self.selectAll)
        
        self.deselectAllButton = QPushButton("Deselect All")
        self.deselectAllButton.clicked.connect(self.deselectAll)

        self.button_hbox = QHBoxLayout()
        self.button_hbox.addWidget(self.selectAllButton)
        self.button_hbox.addWidget(self.deselectAllButton)
        self.button_hbox.addStretch(1)
        
        self.selectAll()

        
    def selectAll(self):
        for checkbox in self.checkboxes:
            checkbox.setChecked(True)

            
    def deselectAll(self):
        for checkbox in self.checkboxes:
            checkbox.setChecked(False)


    def collectCheckedBoxes(self):

        checked_boxes = []        
        for checkbox in self.checkboxes:
            if checkbox.isChecked():
                checked_boxes.append(checkbox.text().split(' ')[0]) # need to strip away the # of records

        return checked_boxes

        
        
# this class is a generic way to build a grid of checkboxes with title and separator line
# need to add a method to get the checked boxes
class SelectionCheckboxGrid(QWidget):

    def __init__(self, checkbox_items, grid_name, orient='col', autocorr=False):
        super().__init__()
        
        self.checkbox_items = checkbox_items
        #self.grid_name = grid_name
        self.num_boxes = len(checkbox_items)
        self.checkboxes = []

        # if we are dealing with baselines, we want to handle the autocorrs
        # with some different methods
        if autocorr:
            first_station = [x[0] for x in self.checkbox_items]
            second_station = [x[1] for x in self.checkbox_items]
            self.auto_idx = []
            for ii in range(self.num_boxes):
                if first_station[ii] == second_station[ii]:
                    self.auto_idx.append(int(ii))

            
        # build a grid of checkboxes

        if self.num_boxes<6:
            cols = 1
        else:
            cols = 6
        rem = self.num_boxes % cols
        if rem==0:
            rows = self.num_boxes // cols
        else:
            rows = self.num_boxes // cols + 1

        self.checkbox_grid = QGridLayout()

        # swap orientation of columns & rows if requested
        if orient=='col':
            positions = [(i, j) for i in range(rows) for j in range(cols)]
        elif orient=='row':
            positions = [(i, j) for i in range(cols) for j in range(rows)]
        else:
            positions = [(i, j) for i in range(rows) for j in range(cols)]
            
        for position, box_name in zip(positions, self.checkbox_items):
            chkbox = QCheckBox(box_name, self)
            chkbox.setCheckable(True)
            self.checkbox_grid.addWidget(chkbox, *position)
            self.checkboxes.append(chkbox)
        
        self.grid_label = QLabel(grid_name)
        self.label_hbox = QHBoxLayout()
        self.label_hbox.addWidget(self.grid_label)
        self.label_hbox.addStretch(1)

        self.separator = QFrame()
        self.separator.setFrameShape(QFrame.HLine)
        self.separator.setLineWidth(1)

        self.selectAllButton = QPushButton("Select All")
        self.selectAllButton.clicked.connect(self.selectAll)
        
        self.deselectAllButton = QPushButton("Deselect All")
        self.deselectAllButton.clicked.connect(self.deselectAll)

        self.button_hbox = QHBoxLayout()
        self.button_hbox.addWidget(self.selectAllButton)
        self.button_hbox.addWidget(self.deselectAllButton)
        

        if autocorr:
            self.selectAutosButton = QPushButton("Select AutoCorrs")
            self.selectAutosButton.clicked.connect(self.selectAutos)
        
            self.deselectAutosButton = QPushButton("Deselect AutoCorrs")
            self.deselectAutosButton.clicked.connect(self.deselectAutos)

            self.button_hbox.addWidget(self.selectAutosButton)
            self.button_hbox.addWidget(self.deselectAutosButton)
            
            
        self.button_hbox.addStretch(1)

        self.selectAll()
        if autocorr:
            self.deselectAutos()

        
    def selectAll(self):
        for checkbox in self.checkboxes:
            checkbox.setChecked(True)

            
    def deselectAll(self):
        for checkbox in self.checkboxes:
            checkbox.setChecked(False)


    def selectAutos(self):
        for ii in self.auto_idx:
            self.checkboxes[ii].setChecked(True)

    def deselectAutos(self):
        for ii in self.auto_idx:
            self.checkboxes[ii].setChecked(False)

    def collectCheckedBoxes(self):

        checked_boxes = []
        for checkbox in self.checkboxes:
            if checkbox.isChecked():
                checked_boxes.append(checkbox.text())

        return checked_boxes




    
# This class sets up the arrangement of checkboxes, textboxes, etc to select the data from the alist file
# It forms the Selection tab
class SelectionPanel(QWidget):

    # Remember, the init block is only run when the window is created
    def __init__(self, alist_data):
        super().__init__()

        self.alist_data = alist_data
        
        # these need to be public variables (self) so they are callable!
        self.param_textbox = SelectionParamBoxGrid(alist_data)
        
        self.baseline_checkboxes = SelectionCheckboxGrid(np.unique(alist_data.records['baseline']), 'Baselines', autocorr=True)

        self.station_checkboxes = SelectionCheckboxGrid(alist_data.unique_stations, 'Stations')
        
        self.qcode_checkboxes = QCodeCheckboxGrid(alist_data.records['qcode'])
        
        self.source_checkboxes = SelectionCheckboxGrid(np.unique(alist_data.records['source']), 'Sources')

        self.pol_checkboxes = SelectionCheckboxGrid(np.unique(alist_data.records['pols']), 'Polarizations')



        # the stretches here do the vertical centering
        # is there a better way to do this with a groupBox?
        select_vbox = QVBoxLayout()
        select_vbox.addLayout(self.param_textbox.params_hbox)
        select_vbox.addWidget(self.param_textbox.separator)
        select_vbox.addLayout(self.param_textbox.textbox_gridv)
        select_vbox.addStretch(1)
        select_vbox.addLayout(self.baseline_checkboxes.label_hbox)
        select_vbox.addWidget(self.baseline_checkboxes.separator)
        select_vbox.addLayout(self.baseline_checkboxes.checkbox_grid)
        select_vbox.addLayout(self.baseline_checkboxes.button_hbox)
        select_vbox.addStretch(1)
        select_vbox.addLayout(self.station_checkboxes.label_hbox)
        select_vbox.addWidget(self.station_checkboxes.separator)
        select_vbox.addLayout(self.station_checkboxes.checkbox_grid)
        select_vbox.addLayout(self.station_checkboxes.button_hbox)
        select_vbox.addStretch(1)
        select_vbox.addLayout(self.qcode_checkboxes.label_hbox)
        select_vbox.addWidget(self.qcode_checkboxes.separator)
        select_vbox.addLayout(self.qcode_checkboxes.checkbox_grid)
        select_vbox.addLayout(self.qcode_checkboxes.button_hbox)
        select_vbox.addStretch(1)
        select_vbox.addLayout(self.source_checkboxes.label_hbox)
        select_vbox.addWidget(self.source_checkboxes.separator)
        select_vbox.addLayout(self.source_checkboxes.checkbox_grid)
        select_vbox.addLayout(self.source_checkboxes.button_hbox)
        select_vbox.addStretch(1)
        select_vbox.addLayout(self.pol_checkboxes.label_hbox)
        select_vbox.addWidget(self.pol_checkboxes.separator)
        select_vbox.addLayout(self.pol_checkboxes.checkbox_grid)
        select_vbox.addLayout(self.pol_checkboxes.button_hbox)
        select_vbox.addStretch(1)


        # add a plot button, connect to a plotting function

        self.setLayout(select_vbox)


    def CollectDataSelections(self):

        data_selection_dict = {}
        
        data_selection_dict['baselines'] = self.baseline_checkboxes.collectCheckedBoxes()
        data_selection_dict['stations'] = self.station_checkboxes.collectCheckedBoxes()
        data_selection_dict['qcodes'] = self.qcode_checkboxes.collectCheckedBoxes()
        data_selection_dict['sources'] = self.source_checkboxes.collectCheckedBoxes()        
        data_selection_dict['pols'] = self.pol_checkboxes.collectCheckedBoxes()
        data_selection_dict['snrrange'] = self.param_textbox.collectParamVals()
        
        return data_selection_dict






# This class sets up the arrangement of checkboxes, textboxes, etc to select the data from the alist file
# It forms the Selection tab
class ScanPanel(QWidget):

    # Remember, the init block is only run when the window is created
    def __init__(self, alist_data):
        super().__init__()

        self.alist_data = alist_data
        
        # these need to be public variables (self) so they are callable!
        self.scan_checkboxes = SelectionCheckboxGrid(np.unique(alist_data.records['scan']), 'Scans')

        select_vbox = QVBoxLayout()
        select_vbox.addLayout(self.scan_checkboxes.label_hbox)
        select_vbox.addWidget(self.scan_checkboxes.separator)
        select_vbox.addLayout(self.scan_checkboxes.checkbox_grid)
        select_vbox.addLayout(self.scan_checkboxes.button_hbox)
        select_vbox.addStretch(1)

        self.setLayout(select_vbox)


    def CollectScanSelections(self):

        scan_selection_dict = {}        
        scan_selection_dict['scans'] = self.scan_checkboxes.collectCheckedBoxes()
        return scan_selection_dict

    




class PlotPanel(QWidget):

    # Remember, the init block is only run when the window is created
    def __init__(self, alist_data, SelectionTab, ScanTab):
        super().__init__()

        self.alist_data = alist_data
        self.SelectionTab = SelectionTab
        self.ScanTab = ScanTab

        # Create a text entry box to load the alist
        plot_label = QLabel('Plot something')
        self.plot_box = QLineEdit()
        self.plot_box.setFixedWidth(300)

        # button to load the file; this will run a mess of python parsing code
        self.plot_button = QPushButton("Plot Data")
        self.plot_button.clicked.connect(self.PlottingButton)
        
        # set up the tab: vertically center an HBox within a VBox
        plot_hbox = QHBoxLayout()
        plot_hbox.addWidget(plot_label)
        plot_hbox.addWidget(self.plot_box)
        plot_hbox.addStretch(1)
        plot_hbox.addWidget(self.plot_button)

        # the stretches here do the vertical centering
        plot_vbox = QVBoxLayout()
        plot_vbox.addStretch(1)
        plot_vbox.addLayout(plot_hbox)
        plot_vbox.addStretch(1)
        
        self.setLayout(plot_vbox)

        #self.show()

    
    def PlottingButton(self, checked):

        # call the plot function from here

        
        data_selection_dict = self.SelectionTab.CollectDataSelections()
        scan_selection_dict = self.ScanTab.CollectScanSelections()

        #print(data_selection_dict['sources'])
        #print(data_selection_dict['qcodes'])
        #print(data_selection_dict['baselines'])
        #print(data_selection_dict['stations'])
        #print(scan_selection_dict['scans'])

        alist_idx = self.alist_data.get_record_indices(data_selection_dict)

        #print(np.unique(np.array(self.alist_data.records['baseline'])[alist_idx]))

        # note, this is not sorted - should we sort it?
        #print(alist_idx)


        # not sure what's the best way to handle the figures
        # send a list of data and labels, order figures by fignum, plot as needed?
        

        #fignum = np.random.randint(0,1000)
        fig = alist_plt.plot_alist_data(1, np.array(self.alist_data.records['scan_time'])[alist_idx],
                                        np.array(self.alist_data.records['amplitude'])[alist_idx],
                                        'Time (Fractional DOY)', 'amplitude (e-4)')

        #plt.figure(fignum)
        plt.show()

        

# Class for the Data window
class DataWindow(QWidget):

    def __init__(self, alist_file, **kwargs):
        super().__init__()

        self.container = QVBoxLayout(self)
        
        self.alist_file = alist_file
    
        self.setWindowTitle('Alist Data')

        # Create a tabs widget in this window
        self.tabs = QTabWidget()
        self.tabs.setTabPosition(QTabWidget.North)
        #self.tabs.setMovable(True)

        self.SelectionTab = SelectionPanel(self.alist_file)
        self.ScanTab = ScanPanel(self.alist_file)
        self.PlotTab = PlotPanel(self.alist_file, self.SelectionTab, self.ScanTab)

        self.tab1 = self.SelectionTab
        self.tab2 = self.ScanTab
        self.tab3 = QWidget() # summary panel
        self.tab4 = self.PlotTab # plot panel
        self.tab5 = QWidget() # calc panel
        self.tab6 = QWidget() # save panel

        
        # Add tabs, specifying the widget in each tab
        self.tabs.addTab(self.tab1, 'Select')
        self.tabs.addTab(self.tab2, 'Scans')
        self.tabs.addTab(self.tab3, 'Summ')
        self.tabs.addTab(self.tab4, 'Plot')
        self.tabs.addTab(self.tab5, 'Calc')
        self.tabs.addTab(self.tab6, 'Save')
        
        #self.setCentralWidget(self.tabs)
        self.container.addWidget(self.tabs)

        # golden aspect ratio
        wsize = 800
        #self.setFixedSize(QSize(wsize,wsize*0.618))
        self.setMinimumSize(QSize(wsize,wsize*1.2))

        #self.setLayout(self.tabs)
        
        #self.show()


        


        



### class for initialization tab
### gets alist filename and spawns data selection window
class Init_tab(QWidget):

    def __init__(self, alist_file=None):
        super().__init__()

        # selection window initialization
        self.w = None

        self.alist_file = alist_file
        
        self.initUI()

        self.show()

        
    def initUI(self):

        # Create a text entry box to load the alist
        init_box_label = QLabel('Alist file')
        self.init_afile_box = QLineEdit()
        if self.alist_file is not None:
            self.init_afile_box.setText(self.alist_file)
        #else:
        #    self.init_afile_box.setText()
        self.init_afile_box.setFixedWidth(300)

        # button to load the file; this will run a mess of python parsing code
        self.init_button = QPushButton("Load Afile")
        self.init_button.clicked.connect(self.load_button)
        
        # set up the tab: vertically center an HBox within a VBox
        init_hbox = QHBoxLayout()
        init_hbox.addWidget(init_box_label)
        init_hbox.addWidget(self.init_afile_box)
        init_hbox.addStretch(1)
        init_hbox.addWidget(self.init_button)

        # the stretches here do the vertical centering
        init_vbox = QVBoxLayout()
        init_vbox.addStretch(1)
        init_vbox.addLayout(init_hbox)
        init_vbox.addStretch(1)
        
        self.setLayout(init_vbox)


    def load_button(self, checked):

        # load the alist file, collect data, open new window with data selection options

        afilename = self.init_afile_box.text()
        
        alist_data = ParseAlist(afilename)

        if self.w is None:
            self.w = DataWindow(alist_data)
        self.w.show()

        #print(alist_data.records.scan_time)
        #print(np.unique(alist_data.records.scan_time))



        

class MainWindow(QMainWindow):
    def __init__(self, **kwargs):
        super().__init__()

        if 'alist_file' in kwargs:
            self.alist_file = kwargs['alist_file']
        else:
            self.alist_file = None
            
            
        self.setWindowTitle("HOPS4 Aedit Test GUI")

        # Create a tabs widget in this window
        self.tabs = QTabWidget()
        self.tabs.setTabPosition(QTabWidget.North)
        #self.tabs.setMovable(True)

        self.tab1 = Init_tab(self.alist_file)
        #self.tab2 = QWidget()
        #self.tab3 = QWidget()
        #self.tab4 = QWidget()
        
        # Add tabs, specifying the widget in each tab
        self.tabs.addTab(self.tab1, 'Init')
        #self.tabs.addTab(self.tab2, 'Select')
        #self.tabs.addTab(self.tab3, 'Plot')
        #self.tabs.addTab(self.tab4, 'Save')
        
        self.setCentralWidget(self.tabs)
        
        # golden aspect ratio
        wsize = 800
        #self.setFixedSize(QSize(wsize,wsize*0.618))
        self.setMinimumSize(QSize(wsize,wsize*0.618))


        
    
if __name__ == '__main__':
    app = QApplication(sys.argv)

    # assume command line argument is an alist filename
    if len(sys.argv)>1:
        window = MainWindow(alist_file=sys.argv[1])
    else:
        window = MainWindow()
    
    window.show()

    app.exec()
